/*
 * communication.h
 *
 *  Created on: Apr 4, 2013
 *      Author: student
 */

#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_


#include <time.h>
#include "order.h"
#include <arpa/inet.h>

#define DATALENGTH 8 // this is the size of the data array passed in the struct


// Opcodes
// {@
#define OPCODE_IMALIVE 		0
#define OPCODE_NEWORDER		1
#define OPCODE_BUTTONHIT	2
#define OPCODE_LIGHT		3
#define OPCODE_ORDERLIST	4
#define OPCODE_ELEVSTATE	5
#define OPCODE_NOOP			6
#define OPCODE_PEERLOSTTAKEOVER	7
#define OPCODE_NEWPEER		8
// @}


#define TOALLIP				0

#define FLOORS 4


struct orders{
	int panel_cmd[FLOORS];
	int panel_up[FLOORS];
	int panel_down[FLOORS];
};

struct msg {
	int msgtype;
	in_addr_t from;
	int to;
	struct orderlist orders;
	int direction;
	int floor;
	int gpdata[DATALENGTH];
};

struct state{
	int floor;
	int direction;
	int internal_state;
};

struct information{
	/* Add the info you would like to have present.
	 *  :information about the others on network
	 * This is used by network.c
	 */
	int active;
	struct state current_state;
	struct orders current_orders;
};

void handle_msg(struct msg package, struct timeval *ttime);
void send_msg(int msgtype, int to, struct orderlist orders, int direction, int floor, int gpdata[]);
/* Parser functions */
char* 	struct_to_byte(	struct msg msg_struct);
struct msg byte_to_struct(char *msg);


#endif /* COMMUNICATION_H_ */
