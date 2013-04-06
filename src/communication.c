/*
 * communication.c
 *
 *  Created on: Apr 4, 2013
 *      Author: student
 */

#include <stdlib.h>
#include "communication.h"
#include <arpa/inet.h>
#include "network.h"
#include "cJSON.h"
#include <time.h>
#include "order.h"

void handle_msg(struct msg package, struct timeval *ttime){
	if(ttime!=0)
		gettimeofday(ttime,0);

	//in_addr_t ip = package.from;
	//struct peer *pp = get(peer_object(0, ip));
	struct elevator elev;
	struct order neworder;

	elev.ip = package.to;
	struct node *n = getelevnode(elev);

	elev.current_state.floor = package.floor;
	elev.current_state.direction = package.direction;
	int floor;
	for(floor = 0; floor < FLOORS; floor ++){
		elev.current_orders[floor][PANEL_CMD] = package.orders[floor][PANEL_CMD];
		elev.current_orders[floor][PANEL_UP] = package.orders[floor][PANEL_UP];
		elev.current_orders[floor][PANEL_DOWN] = package.orders[floor][PANEL_DOWN];
	}



	switch (package.msgtype){
	case OPCODE_NEWPEER:
		elev.ip = package.from;
		addelev(elev);
		break;
	case OPCODE_IMALIVE:

		break;
	case OPCODE_NEWORDER:
		neworder.floor 		= package.gpdata[0];
		neworder.paneltype	= package.gpdata[1];
		n->elevinfo.current_orders[package.gpdata[0]][package.gpdata[1]] = 1;
		printf("Got new order for ip: %i\n", n->elevinfo.ip);
//		order_add_order(neworder);
		//n.elevinfo = elev;

		//addorder(getelevnode(n), neworder);
		break;
	case OPCODE_BUTTONHIT:

		break;
	case OPCODE_LIGHT:

		break;
	case OPCODE_ORDERLIST:

		break;
	case OPCODE_ELEVSTATE:

		break;
	case OPCODE_NOOP:

		//printf("Noop received.\n");

		break;
	case OPCODE_PEERLOSTTAKEOVER:
		printf("Redirect tasks of the lost peer(ip:%i)\n", package.from);
		elev.ip = package.from;
		rmelev(elev);
		break;
	case OPCODE_PEERLOST:
		elev.ip = package.from;
		rmelev(elev);
		break;
	}
}

void send_msg(int msgtype, in_addr_t to, int orders[][N_PANELS], int direction, int floor, int gpdata[]){
	struct msg packet = {
			.msgtype = msgtype,
			.to = to,
			.direction = direction,
	};
	int flooriter;
	for(flooriter = 0; flooriter<FLOORS; flooriter++){
		packet.orders[flooriter][PANEL_CMD] = orders[flooriter][PANEL_CMD];
		packet.orders[flooriter][PANEL_UP]  = orders[flooriter][PANEL_UP];
		packet.orders[flooriter][PANEL_DOWN] = orders[flooriter][PANEL_DOWN];
	}

	int i;
	for (i = 0; i < sizeof(gpdata); i ++){
		packet.gpdata[i] = gpdata[i];
	}
	sendtoallpeer(packet);
}





/*
 * cJSON parser functions
 */
char * struct_to_byte(struct msg msg_struct){
	cJSON *root, *msgtype, *from, *to, *gpdata, *panel_cmd, *panel_up, *panel_down, *direction, *floor;

	root 		= cJSON_CreateObject();
	msgtype 	= cJSON_CreateObject();
	from 		= cJSON_CreateObject();
	to 			= cJSON_CreateObject();
	panel_cmd	= cJSON_CreateObject();
	panel_up 	= cJSON_CreateObject();
	panel_down	= cJSON_CreateObject();
	gpdata 		= cJSON_CreateObject();
	direction	= cJSON_CreateObject();
	floor		= cJSON_CreateObject();

	cJSON_AddNumberToObject(root, "msgtype"	, msg_struct.msgtype);
	cJSON_AddNumberToObject(root, "from"	, msg_struct.from	);
	cJSON_AddNumberToObject(root, "to"		, msg_struct.to		);
	cJSON_AddNumberToObject(root, "direction",msg_struct.direction);
	cJSON_AddNumberToObject(root, "floor"	, msg_struct.floor	);


	cJSON_AddItemToObject(root, "panel_cmd"	, panel_cmd);
	cJSON_AddItemToObject(root, "panel_up"	, panel_up);
	cJSON_AddItemToObject(root, "panel_down", panel_down);
	int flooriter;
	for(flooriter = 0; flooriter<FLOORS; flooriter++){
		cJSON_AddNumberToObject(panel_cmd, "order", msg_struct.orders[flooriter][PANEL_CMD]);
		cJSON_AddNumberToObject(panel_up, "order", msg_struct.orders[flooriter][PANEL_UP]);
		cJSON_AddNumberToObject(panel_down, "order", msg_struct.orders[flooriter][PANEL_DOWN]);
	}

	cJSON_AddItemToObject(	root, "gpdata"		, gpdata);
	int i;
	for(i = 0; i < DATALENGTH; i++){

		cJSON_AddNumberToObject(gpdata, "int", msg_struct.gpdata[i]);
	}

	char * msg = cJSON_Print(root);
	return(msg);
}

struct msg byte_to_struct(char *mesg){
	struct msg msg_struct;
	//if(strlen(mesg)>256){
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

			cJSON * panel_cmd	= cJSON_GetObjectItem(root, 	"panel_cmd");
			cJSON * panel_up	= cJSON_GetObjectItem(root, 	"panel_up");
			cJSON * panel_down	= cJSON_GetObjectItem(root, 	"panel_down");
			cJSON * cmditer		= cJSON_GetObjectItem(panel_cmd,"order");
			cJSON * upiter		= cJSON_GetObjectItem(panel_up, "order");
			cJSON * downiter	= cJSON_GetObjectItem(panel_down,"order");
			int flooriter;
			for (flooriter= 0; flooriter<FLOORS; flooriter++){
				msg_struct.orders[flooriter][PANEL_CMD] 	= cmditer->valueint;
				msg_struct.orders[flooriter][PANEL_UP] 	= upiter->valueint;
				msg_struct.orders[flooriter][PANEL_DOWN] 	= downiter->valueint;
				cmditer = cmditer->next;
				upiter = upiter->next;
				downiter = downiter->next;
			}

			cJSON * gpdata 	= cJSON_GetObjectItem(root, "gpdata");
			cJSON * dataiter= cJSON_GetObjectItem(gpdata, "int");

			int i;
			for(i = 0; i < DATALENGTH; i++){
				msg_struct.gpdata[i] = dataiter->valueint;
				dataiter = dataiter->next;
			}
			return(msg_struct);
		}
	}
	msg_struct.msgtype = OPCODE_CORRUPT;
	return msg_struct;
}
