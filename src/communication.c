/*
 * communication.c
 *
 *  Created on: Apr 4, 2013
 *      Author: student
 */

#include <stdlib.h>
#include "communication.h"
#include "network.h"
#include "cJSON.h"
#include <time.h>

void handle_msg(struct msg package, struct timeval *ttime){
	gettimeofday(ttime,0);
	switch (package.msgtype){
	case OPCODE_IMALIVE:
		printf("excmsg: I'm alive\n");
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
	}
}

void send_msg(int msgtype, int to, int data[]){
	struct msg packet = {
			.msgtype = msgtype,
			.to = to
	};
	int i;
	for (i = 0; i < sizeof(data); i ++){
		packet.data[i] = data[i];
	}
	sendtoallpeer(packet);
}

/*void send_my_state(){
	int msgtype = OPCODE_ELEVSTATE;
	int data[DATALENGTH];

	data[FLOOR_NR_INDEX] = current_floor();
	data[DIRECTION_INDEX]= current_direction();
	data[INTERNAL_STATE_INDEX] = my_state();
}*/





/*
 * cJSON parser functions
 */
char * struct_to_byte(struct msg msg_struct){
	cJSON *root, *msgtype, *from, *to, *data;

	root 	= cJSON_CreateObject();
	msgtype = cJSON_CreateObject();
	from 	= cJSON_CreateObject();
	to 		= cJSON_CreateObject();
	data 	= cJSON_CreateObject();

	cJSON_AddNumberToObject(root, "msgtype"	, msg_struct.msgtype);
	cJSON_AddNumberToObject(root, "from"	, msg_struct.from	);
	cJSON_AddNumberToObject(root, "to"		, msg_struct.to		);
	cJSON_AddItemToObject(	root, "data"	, data);
	int i;
	for(i = 0; i < DATALENGTH; i++){

		cJSON_AddNumberToObject(data, "int", msg_struct.data[i]);
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
	cJSON * data 	= cJSON_GetObjectItem(root, "data");
	cJSON * dataiter= cJSON_GetObjectItem(data, "int");
	msg_struct.msgtype 	= msgtype->valueint	;
	msg_struct.from	   	= from->valueint	;
	msg_struct.to		= to->valueint		;
	int i;
	for(i = 0; i < DATALENGTH; i++){
		msg_struct.data[i] = dataiter->valueint;
		dataiter = dataiter->next;
	}
	return(msg_struct);
}
