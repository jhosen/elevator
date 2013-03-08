/*
 *  network.c
 *  elevator
 *
 *  Created by Jesper Hosen & Dag Sletteboe on 23.02.13.
 *  Copyright 2013 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <ifaddrs.h>
#include <netdb.h>
#include "network.h"
			

#define TRUE  1
#define FALSE 0



// Buffers for send and transmitting data via network
// {@

static  buffer_t buf_in;
		//buf_in.buf = malloc(BUFFER_SIZE);
static  buffer_t buf_out;

// @}

static int listen_socket;


void network_init(void){
	// Create file for keeping track of connected peers.
	FILE * connected_peers = fopen("connected_peers.txt", "w");
	const char * my_ip = getlocalip();
	printf("My IP: %s\n",my_ip);
	fprintf(connected_peers, my_ip);
	fprintf(connected_peers, "\n");
	fclose(connected_peers);


	
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
		printf("Some Peer initiated a new TCP connection to me.\n");

		char * peer_ip = inet_ntoa(peer.sin_addr);
		printf("peer_ip : %s\n", peer_ip);
		if(!is_connected(peer_ip)){
			FILE * connected_peers = fopen("connected_peers.txt", "a+");
			fprintf(connected_peers, peer_ip);
			fprintf(connected_peers, "\n");
			fclose(connected_peers);
			assign_com_thread(new_peer_socket, peer_ip );
		}
		else
			close(new_peer_socket);
	}
}






int connect_to_peer(in_addr_t peer_ip){
	int peer_socket = socket(AF_INET, SOCK_STREAM, 0); 
	
	struct sockaddr_in peer; 
	peer.sin_family			= AF_INET;
	peer.sin_addr.s_addr	= peer_ip;
	peer.sin_port			= htons(LISTEN_PORT); // Connect to listen port.  
	if (connect(peer_socket, (struct sockaddr *)&peer , sizeof(peer)) < 0){
		perror("err: connect. Connecting to peer failed\n");
		return -1; 
	}
	printf("peer_socket : %i, peer_ip : %s\n", peer_socket, inet_ntoa(peer.sin_addr));

	assign_com_thread(peer_socket, inet_ntoa(peer.sin_addr));
	// Add to list off peers
	
	return 1; 
}


struct peer_info {
	int socket;
	char * ip;
};
/* \!brief Assign a thread for handling one dedicated connection. 
 *
 */
void assign_com_thread(int peer_socket, char* peer_ip){
	// We have a connection! Now assign a communication handler thread. 

	struct peer_info *pinf = malloc(sizeof(struct peer_info));
	pinf->socket 	= peer_socket;
	pinf->ip		= peer_ip;
//	printf("peer_socket : %i, peer_ip : %s\n", pinf->socket, pinf->ip);

	pthread_t com_thread;
	
	if( pthread_create( &com_thread , NULL ,  com_handler , pinf)<0){// peer_socket_p) < 0){
		perror("err: pthread_create\n");
		exit(1);
	}
}



/* \!brief Communication handler
 *
 */
void *com_handler(void * peer_inf){
	// The connection is established. This is the function describing what to communicate.
	struct peer_info* pinf = (struct peer_info*) peer_inf;
	int peer_socket = pinf->socket;
	char * peer_ip  = pinf->ip;

	printf("New communication handler thread created for peer connected to socket %d \n", peer_socket);
	printf("peer_socket : %i, peer_ip : %s\n", peer_socket, peer_ip);

	pthread_t send;
    	if(pthread_create(&send, NULL, &func_send, peer_inf)) {
		printf("Could not create thread\n");
		return -1;
   	 }

   

	pthread_t receive;
    	if(pthread_create(&receive, NULL, &func_receive, peer_inf)) {
		printf("Could not create thread\n");
		return -1;
 	}
   	pthread_join(receive, NULL);
   	pthread_join(send, NULL);
	printf("Kill com handler thread\n");

   	pthread_exit(0);
}


void *func_receive(void * peer_inf){
	char rec_msg[2000];
	int read_size;

	struct peer_info* pinf = (struct peer_info*) peer_inf;
	int peer_socket = pinf->socket;
	char * peer_ip  = pinf->ip;

	while(1){
		if((read_size=recv(peer_socket, rec_msg, 2000, 0))<=0){
			perror("err:recv. Receive failed.\n");
			if (read_size == 0){
				perror("err:peer disconnected.\n");
				#warning "Remove peer and socket from list, and close socket connection";
				printf("peer_socket : %i, peer_ip : %s\n", peer_socket, peer_ip);
				rm_from_connected_list(peer_ip);
				close(peer_socket);
				perror("Closed socket, terminating thread\n");
				pthread_exit(0);
			}
			else if (read_size==-1){
				perror("err:received failed\n.");
			}

		}	
		else{
			put_to_buf(rec_msg, buf_in);
			memset(rec_msg, 0, sizeof(rec_msg)); // flush network receive buffer  
		}
	}
}

void *func_send(void *peer_inf){
	struct peer_info* pinf = (struct peer_info*) peer_inf;
	int peer_socket = pinf->socket;
	char * peer_ip  = pinf->ip;
	while(1){
		// Check for new data in buf_out
		if(send(peer_socket, "I'm sending you an SMS.", 25, NULL)<=0){
			printf("Kill send thread\n");
			pthread_exit(0);
		}

		sleep(2);
		; 
	}
}


void put_to_buf(char * value, buffer_t buf){
	while(buf.unread){ // make sure "brain" has collected information from buffer, to avoid overwriting.
		#warning "One should add a timeout for this while loop to avoid infinte hanging"
		; // Do wait. 
	}
	pthread_mutex_lock(&buf_in.mutex);

	//buf.buf = *value;
	printf("Received: %s\n", value);
	buf.unread = 1;
	pthread_mutex_unlock(&buf_in.mutex);
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

	while(1) {
		if (recvfrom(sock, messager, 50 , 0, (struct sockaddr *)&their_addr, &addr_len) == -1) {
			perror("recvfrom");
			exit(1);
		}
		// Check if ip is myself or already connected. 
		char * peer_ip = inet_ntoa(their_addr.sin_addr);
		if(!is_connected(peer_ip)){
			FILE * connected_peers = fopen("connected_peers.txt", "a+");
			fprintf(connected_peers, peer_ip);
			fprintf(connected_peers, "\n");
			fclose(connected_peers);

			if( connect_to_peer(their_addr.sin_addr.s_addr)==-1){
				perror("err: connect_to_peer.\n Error when trying to initate a new connection to a peer by TCP\n");
			}
			else{
				printf("Successful connected to a new peer by TPC after I received an UDP broadcast.\n");
			}
		}
		else{
			NULL;
		}
	}
}

int is_connected(char * peer_ip){
	FILE *connected_peers = fopen("connected_peers.txt", "r+");

	char peer_ip_str[128];
	size_t len = 0;
	char * line = NULL;
	struct in_addr new_peer_struct, old_peer_struct;
	while (getline(&line, &len, connected_peers)!= -1){
		inet_aton(line, &old_peer_struct);
		inet_aton(peer_ip, &new_peer_struct);
		if (new_peer_struct.s_addr == old_peer_struct.s_addr){
			fclose(connected_peers);
			return 1;
		}
	}
	fclose(connected_peers);
	return 0;
}
void rm_from_connected_list(char *peer_ip){
	FILE *connected_peers = fopen("connected_peers.txt", "r+");
	FILE *tmp_file = fopen("temp.txt", "w");
	size_t len = 0;
	char * line = NULL;
	printf("Deleting peer with ip: %s\n", peer_ip);
	struct in_addr delete_peer, iterate_peer;
	inet_aton(peer_ip, &delete_peer);
	while (getline(&line, &len, connected_peers)!= -1){
		inet_aton(line, &iterate_peer);
		if (delete_peer.s_addr == iterate_peer.s_addr){
			printf("Removed peer from list\n");
		}
		else{
			fprintf(tmp_file, line);
			//fprintf(tmp_file, "\n");
		}
	}
	fclose(tmp_file);
	fclose(connected_peers);

	// COPY from temp to con_peers list
	connected_peers = fopen("connected_peers.txt", "w");
	tmp_file = fopen("temp.txt", "r");
	while (getline(&line, &len, tmp_file)!= -1){
		fprintf(connected_peers, line);
	}
	fclose(tmp_file);
	fclose(connected_peers);

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
	toAddr.sin_addr.s_addr = inet_addr(LAN_BROADCAST_IP);
	toAddr.sin_port        = htons(UDP_LISTEN_PORT);
	
	char *message = "Hello, World!";
	while(1) {
		if (sendto(sock, message, strlen(message), 0,(struct sockaddr *) &toAddr, sizeof(toAddr))<0) {
			perror("sendto");
			exit(1);
		}
		else {
			NULL;
		}
		sleep(3);
	}
	
}

char* getlocalip() {
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }


    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;

        s=getnameinfo(ifa->ifa_addr,sizeof(struct sockaddr_in),host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

        if((strcmp(ifa->ifa_name,"eth0")==0)&&(ifa->ifa_addr->sa_family==AF_INET))
        {
            if (s != 0)
            {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }
            printf("\tInterface : <%s>\n",ifa->ifa_name );
            printf("\t  Address : <%s>\n", host);
            freeifaddrs(ifaddr);
            return host;
        }
    }

    freeifaddrs(ifaddr);
    return -1;
}



/* Linked list for keeping track of connected peers
 *
 *
 */

struct peer {
	int socket; 
	int ip;
};

struct node {
	struct peer p;
	struct node *next, *prev;
};

static struct node *root;  

void initlist(){ //struct peer *root
	root = malloc( sizeof(struct node) );  
  	root->p.socket 	= 0; 
    root->p.ip 		= 0;
    root->next 		= 0;  
    root->prev		= 0;  
}

int add(struct peer new){
	struct node * iter, *prev; 
	iter = root; 
	if(iter!=0){
		while(iter->next!=0){
			iter = iter->next; 
		}
	}
	iter->next = malloc(sizeof(struct node));
	prev = iter; 
	iter = iter->next; 
	if(iter==0){
		return 0; //out of memory
	}
	iter->p.socket 	= new.socket; 	
	iter->p.ip	 	= new.ip; 	
	iter->next  	= 0; 
	iter->prev		= prev; 
	return 1; // success
	
}

int rm(struct peer p){
	struct node * iter, *prev, *tmp; 
	iter = root; 
	if(iter!=0){
		while(iter!=0){
			if((iter->p.ip) == p.ip && (iter->p.socket)==p.socket){
				tmp = iter; 
				iter->prev->next = iter->next; 
				iter->next->prev = iter->prev; 
				free(tmp); 
				return 1; 
			}
			prev = iter; 	
			iter = iter->next; 
		}
	}
	return 0;
}