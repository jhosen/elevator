/*
 *  network.h
 *  elevator
 *
 *  Created by Jesper Hosen and Dag Sletteboe on 23.02.13.
 *  Copyright 2013 __MyCompanyName__. All rights reserved.
 *
 */
#include <arpa/inet.h>
#include <pthread.h>
#include "utils/cb_buffer.h"
#include "utils/statemachine.h"
#include "communication.h"

#define LAN_BROADCAST_IP	"129.241.187.255"

#define MAXRECVSIZE 		4096	// Maximum number of bytes allowed to send and receive in one bulk
#define BUFFER_SIZE			128		// Maximum number of elements alowed to buf up in send queue for peer
#define LISTEN_BACKLOG		5		// Number of peers allowed in listen queue.

#define LISTEN_PORT			3000 	// Ports must be between 1024 and 65535
#define UDP_LISTEN_PORT		3002


#define IMALIVE_TIMEOUT		3 					// [seconds]
#define PINGPERIOD 			0.1 				// [seconds]
#define IMALIVE_UPPERIOD 	100000 				// [microseconds]
#define BROADCAST_PERIOD	1					// [seconds]


#define N_NW_STATES 3
#define N_NW_EVENTS 4
#define NW_BC_TIME  3


typedef enum nw_event_t {TIMEOUT_BC = 1, CONNECTION, DISCONNECTION};
typedef enum nw_state_t {INIT, ALONE, ONLINE};

struct peer {
	int socket;
	int active;
	in_addr_t ip;
	CircularBuffer bufout;
	pthread_t com_thread;
};

struct nw_node {
	struct peer p;
	struct nw_node *next, *prev;
};

/* \!brief Function initalizing this peer before enter the network
 *
 */
void network_init(void);

void *network_statemachine();

void network();

/* \!brief Communication handler
 * The connection is established. This is the function describing how to communicate
 */
void *com_handler(void * peer_inf);

/* !\brief Add struct msg to out-buffer for all peers.
 * This data will be pushed out on socket by com_handler.
 *
 */
int sendtoallpeer(struct msg package);
int sendtopeer(struct msg package, struct peer p);

/* \!brief Connect to peer.
 * This is called from listen_udp thread
 *
 * \retval Whether connecting went ok or not (1/0).
 */
int connect_to_peer(in_addr_t peer_ip);

/* \!brief Assigning a communication handler thread to a connection
 *
 */
void assign_com_thread(struct peer p);

void* listen_udp_broadcast();

void* send_udp_broadcast();

/* \!brief Listen for incomming connections
 *
 */
void *listen_tcp();


static void *start_timer();
static int 	isalone();
static void startlisten_tcp();
static void stoplisten_tcp();
static void startlisten_udp();
static void stoplisten_udp();
static void startbroadcast_udp();
static void stopbroadcast_udp();


void nw_setevent(events_t evnt);
events_t nw_getevent();

char* getlocalip();

/* List functions for keeping track of connected peers */
static void nw_initlist();

static struct peer peer_object(int socket, in_addr_t ip);

static int nw_add(struct peer new);

static int nw_rm(struct peer p);

static int nw_find(struct peer p);

static in_addr_t highest_ip();

static struct peer * nw_get(struct peer p);

static int nw_count();




