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

void handle_msg(struct msg package, struct timeval *ttime){
	if(ttime!=0)
		gettimeofday(ttime,0);

	//in_addr_t ip = package.from;
	//struct peer *pp = get(peer_object(0, ip));

	switch (package.msgtype){
	case OPCODE_IMALIVE:

		break;
	case OPCODE_NEWORDER:

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

		printf("Noop received.\n");

		break;
	case OPCODE_PEERLOSTTAKEOVER:
		printf("Redirect tasks of the lost peer(ip:%i)\n", package.from);

		break;
	}
}

void send_msg(int msgtype, int to, struct orders orderlist, int direction, int floor, int gpdata[]){
	struct msg packet = {
			.msgtype = msgtype,
			.to = to,
			.direction = direction,
	};
	int flooriter;
	for(flooriter = 0; flooriter<FLOORS; flooriter++){
		packet.orderlist.panel_cmd[flooriter] = orderlist.panel_cmd[flooriter];
		packet.orderlist.panel_up[flooriter]  = orderlist.panel_up[flooriter];
		packet.orderlist.panel_down[flooriter]= orderlist.panel_down[flooriter];
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
		cJSON_AddNumberToObject(panel_cmd, "order", msg_struct.orderlist.panel_cmd[flooriter]);
		cJSON_AddNumberToObject(panel_up, "order", msg_struct.orderlist.panel_up[flooriter]);
		cJSON_AddNumberToObject(panel_down, "order", msg_struct.orderlist.panel_down[flooriter]);
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
	cJSON *root = cJSON_Parse(mesg);

	cJSON * msgtype = cJSON_GetObjectItem(root, "msgtype");
	cJSON * from 	= cJSON_GetObjectItem(root, "from");
	cJSON * to	 	= cJSON_GetObjectItem(root, "to");
	cJSON * direction=cJSON_GetObjectItem(root, "direction");
	cJSON * floor	=cJSON_GetObjectItem(root, "floor");

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
		msg_struct.orderlist.panel_cmd[flooriter] 	= cmditer->valueint;
		msg_struct.orderlist.panel_up[flooriter] 	= upiter->valueint;
		msg_struct.orderlist.panel_down[flooriter] 	= downiter->valueint;
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
