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
#define OPCODE_CORRUPT		9
#define OPCODE_PEERLOST		10
#define OPCODE_ORDERDONE	11
// @}


#define TOALLIP				0

#define FLOORS 4


struct msg {
	int msgtype;
	in_addr_t from;
	int to;
	int orders[FLOORS][N_PANELS];
	int direction;
	int floor;
	int gpdata[DATALENGTH];
};

struct state{
	int floor;
	int direction;
	int internal_state;
};


void handle_msg(struct msg package, struct timeval *ttime);
void send_msg(int msgtype, int to,  int orders[][N_PANELS], int direction, int floor, int gpdata[]);
/* Parser functions */
char* 	struct_to_byte(	struct msg msg_struct);
struct msg byte_to_struct(char *msg);


#endif /* COMMUNICATION_H_ */
