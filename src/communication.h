/*
 * communication.h
 *
 *  Created on: Apr 4, 2013
 *      Author: student
 */

#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_


#include <time.h>

#define DATALENGTH 8 // this is the size of the data array passed in the struct
#define MSGSIZE 1448 // this depends on the format of the struct

// Opcodes
// {@
#define OPCODE_IMALIVE 		0
#define OPCODE_NEWORDER		1
#define OPCODE_BUTTONHIT	2
#define OPCODE_LIGHT		3
#define OPCODE_ORDERLIST	4
#define OPCODE_ELEVSTATE	5
#define OPCODE_NOOP			6
// @}


#define TOALLIP				0

struct msg {
	int msgtype;
	int from;
	int to;
	int data[DATALENGTH];
};

struct state{
	int floor;
	int direction;
	int internal_state;
};

#define FLOORS 4

struct orders{
	int panel_cmd[FLOORS];
	int panel_up[FLOORS-1];
	int panel_down[FLOORS-1];
};

struct elevator{
	//struct peer p;
	struct state current_state;
	struct orders current_orders;
};

void handle_msg(struct msg package, struct timeval *ttime);
void send_msg(int msgtype, int to, int data[]);


/* Parser functions */
char* 	struct_to_byte(	struct msg msg_struct);
struct msg byte_to_struct(char *msg);


#endif /* COMMUNICATION_H_ */
