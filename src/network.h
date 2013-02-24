/*
 *  network.h
 *  elevator
 *
 *  Created by Jesper Hosen on 23.02.13.
 *  Copyright 2013 __MyCompanyName__. All rights reserved.
 *
 */
#include <arpa/inet.h>





#define BUFFER_IN_SIZE		128
#define BUFFER_OUT_SIZE		128
#define LISTEN_BACKLOG		5		// number of peers allowed in listen queue.
#define LISTEN_PORT			3010	// Ports must be between 1024 and 65535
#define UDP_SEND_PORT		3011
#define UDP_LISTEN_PORT		3012

#define LAN_BROADCAST_IP 	"129.241.187.255"


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
void *network_listen_for_incoming_and_accept();

/* \!brief Communication handler thread
 *
 */
void *com_handler(void * peer_socket_p);

/* \!brief Connect to peer. 
 * This is called from listen_udp thread
 *
 * \retval Whether connecting went ok or not (1/0).
 */
int connect_to_peer(in_addr_t peer_ip);
/* \!brief Assigning a communication handler thread to a connection
 *
 */
void assign_com_thread(int peer_socket);


/*
 * Checks whether peer with peer_ip already is connected.
 */
int is_connected(char * peer_ip);

char* getlocalip();

void* listen_udp_broadcast();

void* send_udp_broadcast();
	
	

