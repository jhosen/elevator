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
#define LISTEN_PORT			3003	// Ports must be between 1024 and 65535
#define UDP_SEND_PORT		3004
#define UDP_LISTEN_PORT		3005


/* \!brief Initiate peer for network cooperation.
 *
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
 */
void connect_to_peer(in_addr_t peer_ip);
/* \!brief Assigning a communication handler thread to a connection
 *
 */
void assign_com_thread(int peer_socket);







void* listen_udp_broadcast();

void* send_udp_broadcast();
	
	

