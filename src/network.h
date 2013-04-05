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
#include "buffer_elev.h"
#include "communication.h"


#define MAXRECVSIZE 		4096
#define BUFFER_SIZE			128
#define LISTEN_BACKLOG		5		// number of peers allowed in listen queue.

#define LISTEN_PORT			3000 //10	// Ports must be between 1024 and 65535
#define UDP_SEND_PORT		3001 //11
#define UDP_LISTEN_PORT		3002 //12

#define LAN_BROADCAST_IP	"129.241.187.255"

#define TIMEOUT 			3 			//1		// [seconds]
#define PINGPERIOD 			0.1 				// [seconds]
#define UPPERIOD 			100000 	// [microseconds]
#define BROADCAST_PERIOD	1					// [seconds]

#define CBUFSIZE 128

/*
typedef struct {
	char buf [MSGSIZE ];
	int unread;
	pthread_mutex_t mutex;
	pthread_barrier_t sync;
} buffer_t;*/

char * getbufin();
void bufout(char *msg);
void bufin(char * value);

struct peer {
	int socket;
	int active;
	in_addr_t ip;
	CircularBuffer bufout;
	struct information info;
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

//void put_to_buf(char * value, buffer_t buf);

int terminate(struct peer p);

/* List functions for keeping track of connected peers */

static void initlist();

static struct peer peer_object(int socket, in_addr_t ip);

static int add(struct peer new);

static int activate(struct peer p);

static int deactivate(struct peer p);

static int rm(struct peer p);

static int find(struct peer p);

static in_addr_t highest_ip();

static struct peer * get(struct peer p);

static int printlist();

static int count();

struct node {
	struct peer p;
	struct node *next, *prev;
};




int sendtoallpeer(struct msg package);
int sendtopeer(struct msg package, struct peer p);

