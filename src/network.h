/*
 *  network.h
 *  elevator
 *
 *  Created by Jesper Hosen on 23.02.13.
 *  Copyright 2013 __MyCompanyName__. All rights reserved.
 *
 */
#include <arpa/inet.h>
#include <pthread.h>




#define BUFFER_SIZE		128
//#define BUFFER_OUT_SIZE		128
#define LISTEN_BACKLOG		5		// number of peers allowed in listen queue.
#define LISTEN_PORT			3000 //10	// Ports must be between 1024 and 65535
#define UDP_SEND_PORT		3001 //11
#define UDP_LISTEN_PORT		3002 //12

//#define JH_BC_IP			"78.91.26.255"
#define LAN_BROADCAST_IP	"129.241.187.255"

#define TIMEOUT 			1	// [sec]
#define PINGPERIOD 			0.1
#define BROADCAST_PERIOD	1



typedef struct {
	char buf [BUFFER_SIZE ];
	int unread;
	pthread_mutex_t mutex;
	pthread_barrier_t sync;
} buffer_t;

char * getbufin();
void bufout(char *msg);
void bufin(char * value);

struct peer {
	int socket;
	in_addr_t ip;
};

/* \!brief Function letting this peer enter the network
 *
 * 1. Bind: one socket for listening for incoming TCP-connections,
 *	  one socket for broadcasting UDP ID-messages,
 *	  one socket for listening for UDP broadcasts.
 * 2. Start UDP-broadcast: send IP-info
 * 3. Listen for connections
 * 4a. response: Accept
 * 4b. timeout: you are only elevator
 */
void network_init(void);

/* \!brief Listen for connections
 *
 */
void *listen_tcp();

/* \!brief Communication handler thread
 *
 */
void *com_handler(void * peer_inf);

/* \!brief Connect to peer. 
 * This is called from listen_udp thread
 *
 * \retval Whether connecting went ok or not (1/0).
 */
int connect_to_peer(in_addr_t peer_ip);
/* \!brief Assigning a communication handler thread to a connection
 *
 */
void assign_com_thread(struct peer p);//int peer_socket, char* peer_ip);


/*
 * Checks whether peer with peer_ip already is connected.
 */
//int is_connected(char * peer_ip);

char* getlocalip();

void* listen_udp_broadcast();

void* send_udp_broadcast();


void *func_receive(void * peer_inf);

void *func_send(void *arg);

void put_to_buf(char * value, buffer_t buf);



/* List functions for keeping track of connected peers */

void initlist();

struct peer peer_object(int socket, in_addr_t ip);

int add(struct peer new);

int rm(struct peer p);

int find(struct peer p);

int printlist();

int count();

struct node {
	struct peer p;
	struct node *next, *prev;
};

