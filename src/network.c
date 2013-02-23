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
	pthread_t listen_tcp_thread, listen_udp_thread, send_udp_broadcast_thread; 
	
	if ( (pthread_create(&listen_tcp_thread, NULL, network_listen_for_incoming_and_accept, (void *) NULL)) < 0){
		perror("err:pthread_create(listen_tcp_thread)\n");
	} 
	
	// We are ready to broadcast
	if ( (pthread_create(&send_udp_broadcast_thread, NULL, send_udp_broadcast, (void *) NULL)) < 0){
		perror("err:pthread_create(send_udp_broadcast_thread)\n");
	} 
	
	// We are ready to listen for broadcast
	if ( (pthread_create(&listen_udp_thread, NULL, listen_udp_broadcast, (void *) NULL)) < 0){
		perror("err:pthread_create(listen_udp_thread)\n");
	} 
	
	
	pthread_join(listen_tcp_thread,			NULL);
	pthread_join(listen_udp_thread,			NULL);
	pthread_join(send_udp_broadcast_thread, NULL);

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






void connect_to_peer(in_addr_t peer_ip){
	int peer_socket = socket(AF_INET, SOCK_STREAM, 0); 
	
	struct sockaddr_in peer; 
	peer.sin_family			= AF_INET;
	peer.sin_addr.s_addr	= peer_ip;//INADDR_ANY; // inet_pton(AF_INET, "ip.ip.ip.ip", NULL);
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



void* listen_udp_broadcast(){
	
	int sock;
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("socket");
		exit(1);
	}
	// Bind my UDP socket to a particular port to listen for incoming UDP responses.
	
	struct sockaddr_in saSocket;
	memset(&saSocket, 0, sizeof(saSocket));
	saSocket.sin_family      = AF_INET;
	saSocket.sin_addr.s_addr = htonl(INADDR_ANY);
	saSocket.sin_port        = htons(UDP_LISTEN_PORT);
	
	int opt = 1;
	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(char*)&opt,sizeof(opt));
	
	if (bind(sock, (struct sockaddr *) &saSocket, sizeof(saSocket)) == 0)
	{
		printf("Socket bound\n");
	}
	else {
		perror("Bind");
	}

	
	struct sockaddr_in their_addr; // connector's address information
	int addr_len =  sizeof(their_addr);
	char messager[50];
	//	int r;
	while(1) {

		if (recvfrom(sock, messager, 50 , 0, (struct sockaddr *)&their_addr, &addr_len) == -1) {
			perror("recvfrom");
			exit(1);
		}
		
		printf("got packet from %s\n",inet_ntoa(their_addr.sin_addr));
		// Check if ip is myself or already connected. 
		connect_to_peer(their_addr.sin_addr.s_addr);
	}
}

void* send_udp_broadcast() {
	
	int sock;
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("socket");
		exit(1);
	}
	
	int opt = 1;
	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(char*)&opt,sizeof(opt));
	int broadcastEnable=1;
	setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
	
	
	struct sockaddr_in toAddr;
	memset(&toAddr, 0, sizeof(toAddr));
	toAddr.sin_family = AF_INET;
	//	toAddr.sin_addr.s_addr = inet_addr("78.91.25.178");
	toAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	toAddr.sin_port        = htons(UDP_LISTEN_PORT);
	
	char *message = "Hello, World!";
	while(1) {
		if (sendto(sock, message, strlen(message), 0,(struct sockaddr *) &toAddr, sizeof(toAddr))<0) {
			perror("sendto");
			exit(1);
		}
		else {
			printf("Sent\n");
		}
		sleep(3);
	}
	
}

