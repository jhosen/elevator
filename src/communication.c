/*
 * communication.c
 *
 *  Created on: Apr 4, 2013
 *      Author: student
 */

#include <stdlib.h>
#include <time.h>
#include <arpa/inet.h>
#include "utils/cJSON.h"
#include "communication.h"
#include "network.h"
#include "order.h"

void handle_msg(struct msg package, struct timeval *ttime){
	if(ttime!=0)
		gettimeofday(ttime,0);
	struct elevator elevto = {.ip = package.to};
	struct elevator elevfrom = {.ip = package.from};
	struct node *nto = getelevnode(elevto);
	struct node *nfrom=getelevnode(elevfrom);
	struct node * lostpeer;
	struct node * processpair;
	int floor, panel, direction;

	switch (package.msgtype){
	case OPCODE_NEWPEER:
        if(nfrom==0){ // Elevator hasn't been connected earlier
                addelev(elevfrom);
        }
		else{    // Elevator has been connected earlier
			activate(gethead(), *nfrom);
			if(nfrom->elevinfo.processpair == gethead()->elevinfo.ip){ // Check if this elevator is the peers process pair
				recover_elev(nfrom);
			}
		}
        nfrom = getelevnode(elevfrom); // This is the new elevator
        sendsyncinfo(gethead(), nfrom);
		break;
	case OPCODE_IMALIVE:

		break;
	case OPCODE_NEWORDER:
		floor = package.gpdata[0];
		panel = package.gpdata[1];
		if(nto!=0)
			order_add_order(nto, floor, panel);
		break;
	case OPCODE_ORDERDONE:
		floor = package.gpdata[0];
		panel = package.gpdata[1];
		if(panel==COMMAND)
			nfrom->elevinfo.current_orders[floor][panel].active = 0;
		else{
			clear_order_all_elev(floor, panel);
		}
		break;
	case OPCODE_ELEVSTATE:
		if(nfrom!=0){
			nfrom->elevinfo.current_state.direction = package.direction;
			nfrom->elevinfo.current_state.floor = package.floor;
		}
		break;
	case OPCODE_PEERLOSTTAKEOVER:
	case OPCODE_PEERLOST:
		lostpeer = nfrom;
		processpair = nto;
		lostpeer->elevinfo.processpair = processpair->elevinfo.ip;
		ordertablemerge(processpair, lostpeer, CALL_UP);
		ordertablemerge(processpair, lostpeer, CALL_DOWN);
		order_flush_panel(lostpeer, CALL_UP);
		order_flush_panel(lostpeer, CALL_DOWN);
		deactivate(gethead(), *lostpeer);
		break;
	case OPCODE_ELEVINEMERGENCY:
		lostpeer = nfrom;
		processpair = nto;
		ordertablemerge(processpair, lostpeer, CALL_UP);
		ordertablemerge(processpair, lostpeer, CALL_DOWN);
		order_flush_panel(lostpeer, CALL_UP);
		order_flush_panel(lostpeer, CALL_DOWN);
		deactivate(gethead(), *lostpeer);
		break;
	case OPCODE_ELEV_NOT_EMERGENCY:
		activate(gethead(), *nfrom);
		break;
	case OPCODE_ELEVSYNC:
		floor = package.floor;
		direction = package.direction;
		getsyncinfo(gethead(),nfrom, package.gpdata, floor, direction);
		break;
	case OPCODE_CORRUPT:
		printf("Received corrupt message\n");
		break;
	case OPCODE_NOOP:


		break;
	}
}

void send_msg(int msgtype, int to, int direction, int floor, int gpdata[]){
	struct msg packet = {
			.msgtype = msgtype,
			.to = to,
			.direction = direction,
			.floor = floor,
	};

	int i;
	for (i = 0; i < sizeof(gpdata); i ++){
		packet.gpdata[i] = gpdata[i];
	}
	sendtoallpeer(packet);
}

/*
 * cJSON parser functions
 */
char * pack(struct msg msg_struct){
	cJSON *root,*gpdata;

	root 		= cJSON_CreateObject();
	gpdata 		= cJSON_CreateObject();

	cJSON_AddNumberToObject(root, "msgtype"	, msg_struct.msgtype);
	cJSON_AddNumberToObject(root, "from"	, msg_struct.from	);
	cJSON_AddNumberToObject(root, "to"		, msg_struct.to		);
	cJSON_AddNumberToObject(root, "direction",msg_struct.direction);
	cJSON_AddNumberToObject(root, "floor"	, msg_struct.floor	);

	cJSON_AddItemToObject(	root, "gpdata"		, gpdata);
	int i;
	for(i = 0; i < DATALENGTH; i++){

		cJSON_AddNumberToObject(gpdata, "int", msg_struct.gpdata[i]);
	}
	char * msg = cJSON_Print(root);
	cJSON_Delete(root);
	return(msg);
}

struct msg unpack(char *mesg){
	struct msg msg_struct;

	cJSON *root = cJSON_Parse(mesg);
	if(root!=0){
		cJSON * msgtype = cJSON_GetObjectItem(root, "msgtype");
		cJSON * from 	= cJSON_GetObjectItem(root, "from");
		cJSON * to	 	= cJSON_GetObjectItem(root, "to");
		cJSON * direction=cJSON_GetObjectItem(root, "direction");
		cJSON * floor	=cJSON_GetObjectItem(root, "floor");

		if(msgtype!=0){
			msg_struct.msgtype 	= msgtype->valueint	;
			msg_struct.from	   	= from->valueint	;
			msg_struct.to		= to->valueint		;
			msg_struct.direction= direction->valueint;
			msg_struct.floor	= floor->valueint	;

			cJSON * gpdata 	= cJSON_GetObjectItem(root, "gpdata");
			cJSON * dataiter= cJSON_GetObjectItem(gpdata, "int");

			int i;
			for(i = 0; i < DATALENGTH; i++){
				msg_struct.gpdata[i] = dataiter->valueint;
				dataiter = dataiter->next;
			}
			cJSON_Delete(root);
			return(msg_struct);
		}
	}
	msg_struct.msgtype = OPCODE_CORRUPT;
	cJSON_Delete(root);
	return msg_struct;
}


/* Returns the index of the last curly bracket.
 * Returns -1 if packet is incomplete
 */
int cjsonendindex(char * cjson_string, int start_i){
	int i, counter, started;
	started = 0;
	counter = 0;
	for(i = start_i; i < strlen(cjson_string); i++){
		if(cjson_string[i]=='{'){
			counter+=1;
			started=1;
		}
		else if(cjson_string[i]=='}' && counter>0){
			counter-=1;
		}
		if(started && counter==0){
			return i;
		}
	}
	return -1;
}


