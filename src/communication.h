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

#define DATALENGTH 	8 // this is the size of the data array passed in the struct
#define TOALLIP		0


typedef enum {OPCODE_IMALIVE, OPCODE_NEWORDER, OPCODE_ELEVSTATE, OPCODE_NOOP, OPCODE_PEERLOSTTAKEOVER,
			OPCODE_PEERLOST, OPCODE_NEWPEER, OPCODE_CORRUPT, OPCODE_ORDERDONE, OPCODE_ELEVINEMERGENCY,
			OPCODE_ELEV_NOT_EMERGENCY, OPCODE_ELEVSYNC} com_opcode_t;






struct msg {
	int msgtype;
	in_addr_t from;
	int to;
	int direction;
	int floor;
	int gpdata[DATALENGTH];
};

struct state{
	int floor;
	int direction;
	int internal_state;
};

/* !\brief This function defines the actions to be done based on opcodes received from network module
 *
 * \param package Keeps information about who sent the command, to whom it is, what should be done etc.
 * \param ttime This is the timeout element to monitor the frequency of I'm alive messages
 *
 */
void handle_msg(struct msg package, struct timeval *ttime);

/* !\brief This function writes messages to peer buffers. This data is later transferred by network module
 *
 * \param msgtype defines which type of message this is (what should be done by the receiving peers).
 * \param to is used to define who this message is meant for (if its for all the TOALLIP should be used).
 * \param orders Used for sending whole order tables.
 * \param direction,
 * \param floor Used when sending state updates
 * \param gpdata General purpose array used to transfer data between peers.
 *
 */
void send_msg(int msgtype, int to, int direction, int floor, int gpdata[]);

/* Parser functions using cJSON */
char* 	pack(struct msg msg_struct);
struct msg unpack(char *msg);

/* !\brief Help function for dividing TCP stream of messages into single packages
 *
 * \param cjson_string char string containing one or more messages
 * \param start_i defines start index of where to look for messages
 */
int cjsonendindex(char * cjson_string, int start_i);

#endif /* COMMUNICATION_H_ */
