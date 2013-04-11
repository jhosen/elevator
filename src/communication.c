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
	struct elevator elevto, elevfrom;
	elevto.ip = package.to;
	elevfrom.ip = package.from;

	struct node *nto = getelevnode(elevto);
	struct node *nfrom=getelevnode(elevfrom);
//	elevfrom.current_state.floor = package.floor;
//	elevfrom.current_state.direction = package.direction;
	int floor;

#warning "Add a test for whether nto and nfrom is 0??";

//	if(nfrom!=0){
//		if(package.msgtype!=0)
//			printf("Received (id _%i_, from: %i)\n", package.msgtype, nfrom->elevinfo.ip);
//	}
	switch (package.msgtype){
	case OPCODE_NEWPEER:
        if(!getelevnode(elevfrom)){ // Elevator hasn't been connected earlier
                addelev(elevfrom);
        }
		else{    // Elevator has been connected earlier
			activate(gethead(), *nfrom);
			recover_elev(nfrom) ;//, nfrom->elevinfo.current_orders); // Recover
		}
		break;
	case OPCODE_IMALIVE:

		break;
	case OPCODE_NEWORDER:
		nto->elevinfo.current_orders[package.gpdata[0]][package.gpdata[1]] = 1;
		//elev_set_button_lamp(package.gpdata[1], package.gpdata[0], 1);
//		printf("Got new order for ip: %i\n", nto->elevinfo.ip);
		break;
	case OPCODE_ORDERDONE:
//		printf("Order at floor: %i, panel: %i was done by %i\n", package.gpdata[0], package.gpdata[1], nfrom->elevinfo.ip);
		nfrom->elevinfo.current_orders[package.gpdata[0]][package.gpdata[1]] = 0;
		clear_order_all_elev(package.gpdata[0], package.gpdata[1]);
		//elev_set_button_lamp(package.gpdata[1], package.gpdata[0], 0); // <--- This is handled by light monitor thread
		break;
	case OPCODE_BUTTONHIT:

		break;
	case OPCODE_LIGHT:

		break;
	case OPCODE_ORDERLIST:

		break;
	case OPCODE_ELEVSTATE:
		nfrom->elevinfo.current_state.direction = package.direction;
		nfrom->elevinfo.current_state.floor = package.floor;
//		printf("elev: %i: \n\tfloor:%i,\n\tdir:%i\n", nfrom->elevinfo.ip, nfrom->elevinfo.current_state.floor, nfrom->elevinfo.current_state.direction);
		break;
	case OPCODE_NOOP:

		break;
	case OPCODE_PEERLOSTTAKEOVER:
//		printf("Redirect tasks of the lost peer(ip:%i)\n", package.from);
//        printf("This elev: \n");
//        order_print_list(nto->elevinfo.current_orders);
//        printf("Lost elev: \n");
//		order_print_list(nfrom->elevinfo.current_orders);
		ordertablemerge(nto->elevinfo.current_orders, nfrom->elevinfo.current_orders, CALL_UP);
		ordertablemerge(nto->elevinfo.current_orders, nfrom->elevinfo.current_orders, CALL_DOWN);
		order_flush_panel(nfrom, CALL_UP);
		order_flush_panel(nfrom, CALL_DOWN);
		//rmelev(elevfrom);
		deactivate(gethead(), *nfrom);
//        printf("This elev after merge: \n");

//        order_print_list(nto->elevinfo.current_orders);
		break;
	case OPCODE_PEERLOST:
	//	rmelev(elevfrom);
//		cleartable(nfrom->elevinfo.current_orders, CALL_UP);
//		cleartable(nfrom->elevinfo.current_orders, CALL_DOWN);
		order_flush_panel(nfrom, CALL_UP);
		order_flush_panel(nfrom, CALL_DOWN);
		deactivate(gethead(), *nfrom);
		break;
	case OPCODE_RECOVER_CMD:
//		int floor;
		if(nto->elevinfo.ip == gethead()->elevinfo.ip){
			for(floor = 0; floor<FLOORS; floor++){
				gethead()->elevinfo.current_orders[floor][COMMAND] |= package.gpdata[floor];
			}
		}
		break;
	case OPCODE_ELEVINEMERGENCY:
		ordertablemerge(nto->elevinfo.current_orders, nfrom->elevinfo.current_orders, CALL_UP);
		ordertablemerge(nto->elevinfo.current_orders, nfrom->elevinfo.current_orders, CALL_DOWN);
		order_flush_panel(nfrom, CALL_UP);
		order_flush_panel(nfrom, CALL_DOWN);
		deactivate(gethead(), *nfrom);
		break;
	case OPCODE_ELEV_NOT_EMERGENCY:
		activate(gethead(), *nfrom);
		break;
	}
}

/* !\brief Function containing recovery routine for elevator reconnected to network.
 *
 *
 */
void recover_elev(struct node * n){//, int orderlist[][N_PANELS]){
	int floor;
	int ordummy[] = {0};
	int cmdorders[FLOORS];
	order_print_list(n->elevinfo.current_orders);
	for(floor = 0; floor < FLOORS; floor ++){
		if(n->elevinfo.current_orders[floor][COMMAND]){
			cmdorders[floor] = 1; // {floor, COMMAND};
			printf("Recover: Sending command floor %i\n", floor);
		}
		else{
			cmdorders[floor] = 0;
		}

	}
	send_msg(OPCODE_RECOVER_CMD, n->elevinfo.ip, ordummy, 0,0, cmdorders);

}
void send_msg(int msgtype, int to, int orders[][N_PANELS], int direction, int floor, int gpdata[]){
	struct msg packet = {
			.msgtype = msgtype,
			.to = to,
			.direction = direction,
			.floor = floor,
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
	cJSON_Delete(root);
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
			cJSON_Delete(root);
			return(msg_struct);
		}
	}
	msg_struct.msgtype = OPCODE_CORRUPT;
	cJSON_Delete(root);
	return msg_struct;
}

//char * getcjsonstr(char *string, int start_i){
//	int end_i = cjsonendindex(string, start_i);
//	while(end_i<(strlen(string)-1)){
//		copy string from start to end
//		start_i = end_i+1;
//		end_i = cjsonendindex(string, start_i);
//
//	}
//	copy string from start to end
////
////	int end_i 	= cjsonendindex(string, start_i);
////
////	char * p = string[start -- end]
////
////	start_i = end_i+1;
////	end_i = cjsonendindex(string, start_i)
//
//
//}

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
		else if(cjson_string[i]=='}'){
			counter-=1;
		}
		if(started && counter==0){
			return i;
		}
	}
	return -1;
}
