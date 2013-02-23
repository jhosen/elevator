/*
 *  network.c
 *  elevator
 *
 *  Created by Jesper Hosen on 23.02.13.
 *  Copyright 2013 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "network.h"

#define TRUE  1
#define FALSE 0

// Buffers for send and transmiting data via network
// {@
static char buf_in [BUFFER_IN_SIZE ];
static char buf_out[BUFFER_OUT_SIZE];
// @}

static int listen_socket;





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
void network_init(void){
	
	int opt = TRUE; 
	struct sockaddr_in listen_addr;
	
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_port = htons(LISTEN_PORT);
	listen_addr.sin_addr.s_addr = htons(INADDR_ANY); // Binding local IP-address
	
	printf("Listen socket config:\n listen_addr.sin_family = %i;\n listen_addr.sin_port = %i;\n listen_addr.sin_addr.s_addr = %i;\n",
		   listen_addr.sin_family ,
		   LISTEN_PORT,
		   listen_addr.sin_addr.s_addr
		   );
	
	listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) == -1){
		perror("err: setsockopt\n");
		exit(1); 
	}
	
	if ( bind(listen_socket, (struct sockaddr *)&listen_addr, sizeof listen_addr) == -1){
		perror("err: bind\n");
		exit(1); 
	}
	
	// Now that the listen socket is bounded, one may start broadcast its present, and listen for incoming connections. 
	// We are ready to listen on listen_socket!
	pthread_t listen_thread; 
	if ( (pthread_create(&listen_thread, NULL, network_listen_for_incoming_and_accept, (void *) NULL)) < 0){
		perror("err:pthread_create(listen_thread)\n");
	} 
	// We are ready to broadcast
	
	
	pthread_join(listen_thread, NULL);

}



/* \!brief Listen for TCP connections and accept incoming. 
 *
 */
void *network_listen_for_incoming_and_accept(){
	
	// Be proactive, create objects for new peer connections:
	struct sockaddr_in peer; 
	int new_peer_socket, c; 
	c = sizeof(struct sockaddr_in);

	while(1){
		listen(listen_socket, LISTEN_BACKLOG);
		if ( (new_peer_socket = accept(listen_socket, (struct sockaddr *)&peer, (socklen_t*)&c)) == -1 ){
			perror("err: accept\n");
			exit(1); 
		}
		printf("New connection\n");

		#warning "Add new peer to list!\n"
		
		assign_com_thread(new_peer_socket); 
	}
}






void connect_to_peer(char * peer_ip){
	int peer_socket = socket(AF_INET, SOCK_STREAM, 0); 
	
	struct sockaddr_in peer; 
	peer.sin_family			= AF_INET;
	peer.sin_addr.s_addr	= inet_addr(peer_ip);//INADDR_ANY; // inet_pton(AF_INET, "ip.ip.ip.ip", NULL);
	peer.sin_port			= htons(LISTEN_PORT); // Connect to listen port.  
	if (connect(peer_socket, (struct sockaddr *)&peer , sizeof(peer)) < 0){
		perror("err: connect. Connectinh to peer failed\n");
		return -1; 
	}

	assign_com_thread(peer_socket);
	// Add to list off peers
	
	return 1; 
}



/* \!brief Assign a thread for handling one dedicated connection. 
 *
 */
void assign_com_thread(int peer_socket){
	// We have a connection! Now assign a communication handler thread. 
	int * peer_socket_p;
	peer_socket_p = (int *)malloc(1);	
	*peer_socket_p = peer_socket; 
	
	pthread_t com_thread;
	
	if( pthread_create( &com_thread , NULL ,  com_handler , (void*) peer_socket_p) < 0){
		perror("err: pthread_create\n");
		exit(1);
	}
}



/* \!brief Communication handler
 *
 */
void *com_handler(void * peer_socket_p){
	// The connection is established. This is the function describing what to communicate.
	int peer_socket = *(int*)peer_socket_p; 
	printf("New communication handler thread created for peer connected to socket %d \n", peer_socket);
	
	// start receiving thread and send thread
	while(1)
		;

}

