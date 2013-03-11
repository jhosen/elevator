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
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include "cJSON.h"
#define TRUE  1
#define FALSE 0



// Buffers for send and transmitting data via network
// {@

static  buffer_t buf_in;
//buf_in.buf = malloc(BUFFER_SIZE);
static  buffer_t buf_out;

// @}
void bufout(char *msg){
	int i = 0;
	while(*(msg+i)!='\0' && i<BUFFER_SIZE){
		buf_out.buf[i] = *(msg+i);
		i++;
	}
	buf_out.unread = 1;
}

static int listen_socket;
static struct node *root;

pthread_mutex_t errmutex;

void network_init(void){
	pthread_mutex_init(&errmutex,0);
	pthread_mutex_init(&buf_in.mutex,0);

	const char *myip  = getlocalip();
	struct in_addr meaddr;
	inet_pton(AF_INET, myip, &meaddr);

	initlist();
	root->p.ip = meaddr.s_addr;



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



	// We are ready to listen on listen_socket!
	pthread_t listen_tcp_thread, listen_udp_thread, send_udp_broadcast_thread;

	// listen for incoming tcp connections
	if ( (pthread_create(&listen_tcp_thread, NULL, listen_tcp, (void *) NULL)) < 0){
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

	/*pthread_join(listen_tcp_thread,			NULL);
	pthread_join(listen_udp_thread,			NULL);
	pthread_join(send_udp_broadcast_thread, NULL);
*/
}



/* \!brief Listen for TCP connections and accept incoming.
 *
 */
void *listen_tcp(){

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
		char * peer_ip = inet_ntoa(peer.sin_addr);
//		printf("Some Peer(ip:%s) initiated a new TCP connection to me.\n", peer_ip);
		struct peer newpeer = peer_object(new_peer_socket, peer.sin_addr.s_addr);
		if(!find(newpeer)){
			add(newpeer);
			assign_com_thread(newpeer);//new_peer_socket, peer_ip); //<--- change
		}
		else{
			printf("Already in connected list\n");
			printlist();
			close(new_peer_socket);
		}
	}
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
		struct peer newpeer = peer_object(0, their_addr.sin_addr.s_addr);
		if(!find(newpeer)){
			if( connect_to_peer(their_addr.sin_addr.s_addr)==-1){
				perror("err: connect_to_peer.\n Error when trying to initate a new connection to a peer by TCP\n");
			}
			else{
//				printf("Successful connected to a new peer by TPC after I received an UDP broadcast.\n");
			}
		}
		else{
			NULL;
		}
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
	toAddr.sin_addr.s_addr = inet_addr(LAN_BROADCAST_IP);
	toAddr.sin_port        = htons(UDP_LISTEN_PORT);

	char *message = "UDP Broadcast!";
	while(1) {
		if (sendto(sock, message, strlen(message), 0,(struct sockaddr *) &toAddr, sizeof(toAddr))<0) {
			perror("sendto");
			exit(1);
		}
		else {
			NULL;
		}
		sleep(BROADCAST_PERIOD);
	}

}





/* \!brief Communication handler
 *
 */
void *com_handler(void * peer){
	// The connection is established. This is the function describing what to communicate.
	struct peer* pinf = (struct peer*) peer;
	struct peer p;
	p.socket = pinf->socket;
	p.ip = pinf->ip;

//	printlist();
//	pthread_barrier_init(&buf_out.sync, NULL, count());


	printf("New communication handler thread created for peer connected to socket %d \n", p.socket);

	char recv_msg[2000];
	char send_msg[2000];
	int read_size;

	time_t timeout = time(0);
	time_t pingtime = clock();
	time_t currenttime;

	int flags;

	/* Set non-blocking state */
	if (-1 == (flags = fcntl(p.socket, F_GETFL, 0))){
		flags = 0;
	}
	fcntl(p.socket, F_SETFL, flags | O_NONBLOCK);

	while(1){
		/* Maintain connection by passing and receiving I'm alive */
//		currenttime = clock();

		if((double)(clock()-pingtime)/CLOCKS_PER_SEC >= PINGPERIOD){
			send(p.socket, "I'm alive", sizeof("I'm alive"), 0);
			pingtime = clock();
		}

		/* Receive data */
		pthread_mutex_lock(&buf_in.mutex);
		read_size = recv(p.socket, recv_msg, 2000, 0);
		pthread_mutex_unlock(&buf_in.mutex);
		if(read_size <= 0){ // Error mode
				if(read_size == 0){
						printf("socket: %i \n", p.socket);
						perror("err:peer disconnected.\n");
						break;
				}
				else if((read_size==-1) ){
					if( (errno != EAGAIN) && (errno != EWOULDBLOCK) ) {
						perror("Receiving failed\n");
						break;
					}
					else{
//						pthread_mutex_unlock(&errmutex);
					}
				}
				else{
					0;// Did not receive anything, but no error
				}
		}
		else {
			timeout = time(0);
			/* Receive data */
			//if(recv_msg!="I'm alive"){
//			printf("msg = %s, read size = %i\n", recv_msg, read_size); // DO SOMETHING
//			bufin(recv_msg);
			//}
//			memset(recv_msg, 0, sizeof(recv_msg)); // flush network receive buffer
		}
		currenttime = time(0);
		if((currenttime-timeout) > TIMEOUT){
			printf("Currtime : % i , timeout : % i\n, if(%i > %i)\n", currenttime, timeout,(currenttime-timeout), TIMEOUT);
			printf("TIMEOUT ON I'M ALIVE, socket: %i\n", p.socket);
			break;
		}
		/* Send data */

		//printf("Waiting for sync on send\n");
//		pthread_barrier_wait(&buf_out.sync);

	}
	terminate(p);
//	printf("\n Connected peers: \n");
//	printlist();
//	printf("\n");
	printf("Kill com handler thread\n");

   	pthread_exit(0);
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

	struct peer p = peer_object(peer_socket, peer_ip);

	add(p);
	assign_com_thread(p);

	return 1;
}



void assign_com_thread(struct peer p){//int peer_socket, char* peer_ip){
	// We have a connection! Now assign a communication handler thread.

	struct peer *pinf = malloc(sizeof(struct peer));
	pinf->socket 	= p.socket;
	pinf->ip		= p.ip;


	pthread_t com_thread;

	if( pthread_create( &com_thread , NULL ,  com_handler , pinf)<0){// peer_socket_p) < 0){
		perror("err: pthread_create\n");
		exit(1);
	}
}

void terminate(struct peer p){
	if(rm(p)){
//		printf("successful removed from list\n");
	}
//	pthread_barrier_destroy(&buf_out.sync);
//	pthread_barrier_init(&buf_out.sync, 0, count()); // update barrier variable

	close(p.socket);
}

//void bufin(char * value){
//	/*while(buf_in.unread){ // make sure "brain" has collected information from buffer, to avoid overwriting.
//		//#warning "One should add a timeout for this while loop to avoid infinte hanging"
//		; // Do wait.
//	}*/
//	pthread_mutex_lock(&buf_in.mutex);
//	int i = 0;
//	while(*(value+i)!='\0' && i<BUFFER_SIZE){
//		buf_in.buf[i] = *(value+i);
//	}
//	buf_in.unread = 1;
//	pthread_mutex_unlock(&buf_in.mutex);
//	printf("Bufin(%s)\n", value);
//}





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




struct peer peer_object(int socket, in_addr_t ip){
	struct peer p;
	p.socket = socket;
	p.ip = ip;
	return p;
}

void initlist(){
	root = malloc( sizeof(struct node) );
  	root->p.socket 	= 0;
    root->p.ip 		= 0;
    root->next 		= 0;
    root->prev		= 0;
}
int printlist(){
	struct node * iter;
	iter = root;
	int i = 0;
	if(iter!=0){
		while(iter!=0){
			printf("Node%i, socket = %i, ip = %i\n", i, iter->p.socket, iter->p.ip);//inet_ntoa(tmp.sin_addr) ); // <-- ERROR
			iter = iter->next;
			i++;
		}
	}
	return 1;
}

int count(){
	struct node * iter;
	iter = root;
	int i = 0;
	if(iter!=0){
		while(iter!=0){
			iter = iter->next;
			i++;
		}
	}
	return i-1; // Do not count yourself
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
				tmp = malloc(sizeof(struct node));
				tmp = iter;
				iter->prev->next = iter->next;
				if(iter->next!=0){
					iter->next->prev = iter->prev;
				}
				free(tmp);
				return 1;
			}
			prev = iter;
			iter = iter->next;
		}
	}
	return 0;
}


int find(struct peer p){
	struct node * iter;
	iter = root;
	if(iter!=0){
		while(iter!=0){
			if((iter->p.ip) == p.ip){
				return 1; // found it
			}
			iter = iter->next;
		}
	}
	return 0;
}


 /*
  * cJSON parser functions
  */


char * struct_to_byte(struct msg msg_struct){
	cJSON *root, *msgtype, *from, *to, *data;

	char* msg;

	root 	= cJSON_CreateObject();
	msgtype = cJSON_CreateObject();
	from 	= cJSON_CreateObject();
	to 		= cJSON_CreateObject();
	data 	= cJSON_CreateObject();

	cJSON_AddNumberToObject(root, "msgtype"	, msg_struct.msgtype);
	cJSON_AddNumberToObject(root, "from"	, msg_struct.from	);
	cJSON_AddNumberToObject(root, "to"		, msg_struct.to		);
	cJSON_AddItemToObject(	root, "data"	, data);
	int i;
	for(i = 0; i < DATALENGTH; i++){

		cJSON_AddNumberToObject(data, "int", msg_struct.data[i]);
	}

	msg = cJSON_Print(root);
	return(msg);
}

struct msg byte_to_struct(char *msg){
	struct msg msg_struct;
	cJSON *root, *msgtype, *from, *to, *data, *dataiter;

	root = cJSON_Parse(msg);
	msgtype = cJSON_GetObjectItem(root, "msgtype");
	from 	= cJSON_GetObjectItem(root, "from");
	to	 	= cJSON_GetObjectItem(root, "to");
	data 	= cJSON_GetObjectItem(root, "data");
	dataiter= cJSON_GetObjectItem(data, "int");

	msg_struct.msgtype 	= msgtype->valueint	;
	msg_struct.from	   	= from->valueint	;
	msg_struct.to		= to->valueint		;
	int i;
	for(i = 0; i < DATALENGTH; i++){
		msg_struct.data[i] = dataiter->valueint;
		dataiter = dataiter->next;
	}
	return(msg_struct);
}

