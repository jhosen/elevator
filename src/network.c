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
#include "buffer_elev.h"
#include "communication.h"

#include "statemachine.h"

#define TRUE  1
#define FALSE 0

#define N_NW_STATES 3
#define N_NW_EVENTS 4
#define NW_BC_TIME  3


static state_t state;
static event_t event;
static sm_config_t config = {.nevents = N_NW_EVENTS, .nstates = N_NW_STATES};

static int listen_socket;
static struct nw_node *root;

pthread_t listen_tcp_thread, listen_udp_thread, send_udp_broadcast_thread, timeout_thread, network_thread;
struct timeval broadcasttime;



static struct state_action_pair_t stateTable[N_NW_STATES][N_NW_EVENTS] = {
    /* state|event:	NOEVENT						TIMEOUT_BC                             	CONNECTION                              DISCONNECTION */
    /*INIT		*/ 	{{INIT,     NULL,   NULL},  {ALONE,     NULL,               NULL},	{ONLINE,	NULL,               NULL},  {INIT,      NULL,               NULL},},
    /*ALONE     */	{{ALONE,    NULL,	NULL},  {ALONE,     NULL,               NULL},	{ONLINE,    stopbroadcast_udp,  NULL},  {ALONE,     NULL,               NULL},},
    /*ONLINE	*/	{{ONLINE,   NULL,	NULL},  {ONLINE,    stopbroadcast_udp,  NULL},	{ONLINE,	NULL,               NULL},	{ALONE,     startbroadcast_udp, isalone} },
};


void network(){
    network_init();
    pthread_create(&network_thread, NULL, network_statemachine, (void *) NULL);
}

void network_init(){
    nw_initlist();
    const char *myip  = getlocalip();
	struct in_addr meaddr;
	inet_pton(AF_INET, myip, &meaddr);
    root->p.ip = meaddr.s_addr;
    pthread_mutex_init(&event.eventMutex, 0);
    state = INIT;
    startlisten_udp();
    startlisten_tcp();
    startbroadcast_udp();
    pthread_create(&timeout_thread, NULL, start_timer, (void *) NULL);
}

void *network_statemachine(){
    while(1){
        statemachine_handleEvent(&stateTable, config, &state, &event);
    }
}



void *com_handler(void * peer){

	struct peer* pinf = (struct peer*) peer;
	struct peer p;
	p.socket = pinf->socket; // creating a local copy of the peer object
	p.ip = pinf->ip;

	struct peer * pp = nw_get(p);

	printf("New communication handler thread created for peer connected to socket %d \n", p.socket);

	struct msg newpeermsg = {.msgtype = OPCODE_NEWPEER,.from	 = p.ip,};
	handle_msg(newpeermsg, 0);

	char recv_msg[MAXRECVSIZE];
	int read_size;
	struct timeval ctime, ptime, ttime;
	gettimeofday(&ttime,0);

	int flags;

	/* Set non-blocking state */
	if (-1 == (flags = fcntl(pinf->socket, F_GETFL, 0))){
		flags = 0;
	}
	fcntl(p.socket, F_SETFL, flags | O_NONBLOCK);

	char * string 	= calloc(MAXRECVSIZE,1);
	char * cjsonstr = calloc(MAXRECVSIZE,1);
	/** Normal operating mode **/
	while(1){
		/** Maintain connection by passing and receiving I'm alive **/
		gettimeofday(&ctime, 0);
		if((ctime.tv_usec - ptime.tv_usec) >= IMALIVE_UPPERIOD || (ctime.tv_usec<ptime.tv_usec)){
			struct msg packet = { .msgtype = OPCODE_IMALIVE,};
			char * cout  = pack(packet);
			send(p.socket, cout, strlen(cout), 0);
			gettimeofday(&ptime, 0);
		}

		/** Receive data **/
		read_size = recv(pinf->socket, recv_msg, MAXRECVSIZE, 0);
		if(read_size <= 0){ // ERROR/RECOVERY mode
				if(read_size == 0){
						printf("err:peer disconnected (socket %i).\n", pinf->socket);
						break;
				}
				else if((read_size==-1) ){
					if( (errno != EAGAIN) && (errno != EWOULDBLOCK) ) {
						perror("Receiving failed\n");
						break;
					}
				}
				else{
					0;// Did not receive anything, but no error
				}
		}
		else { // Received something
			strcpy(string, recv_msg);
			string[read_size]='\0';

			int start_i = 0;
			int end_i = cjsonendindex(string, start_i);
			/* Separate packets arriving together */
			while(end_i<(strlen(string)-1)){
				strncpy(cjsonstr, (string+start_i), (end_i-start_i+1));
				cjsonstr[end_i-start_i+1] = '\0';
				struct msg packetin = unpack(cjsonstr);
				packetin.from = p.ip;
				handle_msg(packetin, &ttime);
				start_i = end_i+1;
				end_i = cjsonendindex(string, start_i);
			}
			if(end_i!=-1){
				strncpy(cjsonstr, (string+start_i), (end_i-start_i+1));
				cjsonstr[end_i-start_i+1] = '\0';
				struct msg packetin = unpack(cjsonstr);
				packetin.from = p.ip;
				handle_msg(packetin, &ttime);
			}
		}
		/** Check for timeout **/
		gettimeofday(&ctime, 0);
		if((ctime.tv_sec-ttime.tv_sec) > IMALIVE_TIMEOUT){
			printf("Timeout on I'm alive, socket: %i\n", pinf->socket);
			break;
		}
		/** Send data **/
		struct msg elem;
		while(!cbIsEmpty(&pp->bufout)){  // Send data from buffer
			 cbRead(&pp->bufout, &elem);
			 if (elem.msgtype!=OPCODE_CORRUPT){
				 char * cout  = pack(elem);
				 send(pinf->socket, cout, strlen(cout), 0);
				 free(cout);
			 }
		}
	}

	/** Recovery mode: **/
	free(cjsonstr);
	free(string);
	nw_rm(p);
    nw_setevent(DISCONNECTION);

	struct msg recovermsg = { .from = p.ip, .to	= highest_ip()};

	if(recovermsg.to == root->p.ip){ // I have the highest ip on the network and should take over orders
		recovermsg.msgtype = OPCODE_PEERLOSTTAKEOVER;
	}
	else{
		recovermsg.msgtype = OPCODE_PEERLOST;
	}
	handle_msg(recovermsg, 0);

	close(p.socket);

	printf("Kill communication handler thread\n");
   	pthread_exit(0);
}



int sendtoallpeer(struct msg package){
	struct peer p = {
			.ip = TOALLIP
	};
	return sendtopeer(package, p);

}

int sendtopeer(struct msg package, struct peer p){
	struct nw_node * iter;
	iter = root;
	iter = iter->next;

	while(iter!=0){
		if((iter->p.ip) == p.ip || p.ip == TOALLIP){
			struct peer * pp = nw_get(iter->p);
			cbWrite(&pp->bufout, &package);
		}
		iter = iter->next;
	}
	return 1;
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
	nw_add(p);
	assign_com_thread(p);
	return 1;
}


void assign_com_thread(struct peer p){
	// We have a connection! Now assign a communication handler thread.
	struct peer *pinf = malloc(sizeof(struct peer));
	pinf->socket 	= p.socket;
	pinf->ip		= p.ip;
	if( pthread_create( &pinf->com_thread , NULL ,  com_handler , pinf)<0){// peer_socket_p) < 0){
		perror("err: pthread_create\n");
		exit(1);
	}
}



/* \!brief Listen for TCP connections and accept incoming.
 *
 */
void *listen_tcp(){
	int opt = TRUE;
	struct sockaddr_in listen_addr;

	listen_addr.sin_family = AF_INET;
	listen_addr.sin_port = htons(LISTEN_PORT);
	listen_addr.sin_addr.s_addr = htons(INADDR_ANY); // Binding local IP-address

	listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) == -1){
		perror("err: setsockopt\n");
		exit(1);
	}
	if ( bind(listen_socket, (struct sockaddr *)&listen_addr, sizeof listen_addr) == -1){
		perror("err: bind\n");
		exit(1);
	}

	struct sockaddr_in peer;
	int new_peer_socket, structsize;
	structsize = sizeof(struct sockaddr_in);

	while(1){
		listen(listen_socket, LISTEN_BACKLOG);
		if ( (new_peer_socket = accept(listen_socket, (struct sockaddr *)&peer, (socklen_t*)&structsize)) == -1 ){
			perror("err: accept\n");
			exit(1);
		}
		struct peer newpeer = peer_object(new_peer_socket, peer.sin_addr.s_addr);
		if(!nw_find(newpeer)){
			nw_add(newpeer);
			assign_com_thread(newpeer);
            nw_setevent(CONNECTION);
		}
		else{ // Already in connected list
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
		if(!nw_find(newpeer)){
			if( connect_to_peer(their_addr.sin_addr.s_addr)==-1){
				perror("err: connect_to_peer.\n Error when trying to initate a new connection to a peer by TCP\n");
			}
			else{
                nw_setevent(CONNECTION);
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


static void *start_timer(){
    struct timeval testtime;
    gettimeofday(&broadcasttime, 0);
    while(1){
        gettimeofday(&testtime, 0);
        if((testtime.tv_sec - broadcasttime.tv_sec)>NW_BC_TIME){
            nw_setevent(TIMEOUT_BC);
            pthread_exit(0);
        }
    }
}



void nw_setevent(events_t evnt){
	pthread_mutex_lock(&event.eventMutex);
	event.event = evnt;
	pthread_mutex_unlock(&event.eventMutex);
}

events_t nw_getevent(){
	events_t evcopy;
	pthread_mutex_lock(&event.eventMutex);
	evcopy = event.event;
	pthread_mutex_unlock(&event.eventMutex);
	return evcopy;
}

static void startlisten_tcp(){
    printf("Starting TCP listen\n");
    // listen for incoming tcp connections
	if ( (pthread_create(&listen_tcp_thread, NULL, listen_tcp, (void *) NULL)) < 0){
		perror("err:pthread_create(listen_tcp_thread)\n");
	}
}

static void stoplisten_tcp(){
    printf("Stopping TCP listen\n");
    pthread_kill(listen_tcp_thread, 0);
}

static void startlisten_udp(){
    printf("Starting UDP listen\n");
	if ( (pthread_create(&listen_udp_thread, NULL, listen_udp_broadcast, (void *) NULL)) < 0){
		perror("err:pthread_create(listen_udp_thread)\n");
	}
}

static void stoplisten_udp(){
    printf("Stopping TCP listen\n");
    pthread_kill(listen_udp_thread, 0);
}

static void startbroadcast_udp(){
    printf("Starting UDP broadcast\n");
	if ( (pthread_create(&send_udp_broadcast_thread, NULL, send_udp_broadcast, (void *) NULL)) < 0){
		perror("err:pthread_create(send_udp_broadcast_thread)\n");
	}
}

static void stopbroadcast_udp(){
    printf("Stopping UDP broadcast\n");
    pthread_kill(send_udp_broadcast_thread, 0);
}


static int isalone(){
    return (nw_count()==0);
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
 */

static struct peer peer_object(int socket, in_addr_t ip){
	struct peer p;
	p.socket = socket;
	p.ip = ip;
	p.active = TRUE;
	return p;
}

static void nw_initlist(){
	root = malloc( sizeof(struct nw_node) );
  	root->p.socket 	= 0;
    root->p.ip 		= 0;
    root->next 		= 0;
    root->prev		= 0;
}


/* !\brief Count the number of connected peers
 *
 */
static int nw_count(){
	struct nw_node * iter;
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

static int nw_add(struct peer new){
	struct nw_node * iter, *prev;
	iter = root;
	if(iter!=0){
		while(iter->next!=0){
			iter = iter->next;
		}
	}
	iter->next = malloc(sizeof(struct nw_node));
	prev = iter;
	iter = iter->next;
	if(iter==0){
		return 0; //out of memory
	}
	iter->p.socket 	= new.socket;
	iter->p.ip	 	= new.ip;
	cbInit(&iter->p.bufout, BUFFER_SIZE);
	iter->next  	= 0;
	iter->prev		= prev;
	return 1; // success

}

static int nw_rm(struct peer p){
	struct nw_node * iter, *prev, *tmp;
	iter = root;
	if(iter!=0){
		while(iter!=0){
			if((iter->p.ip) == p.ip && (iter->p.socket)==p.socket){
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


static int nw_find(struct peer p){
	struct nw_node * iter;
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

static in_addr_t highest_ip(){
	struct nw_node * iter;
	iter = root;
	in_addr_t highest = 0;
	while(iter!=0){
		if((iter->p.ip) > highest){
				highest = iter->p.ip;
		}
		iter = iter->next;
	}
	return highest;
}

static struct peer * nw_get(struct peer p){
	struct nw_node * iter;
	iter = root;
	if(iter!=0){
		while(iter!=0){
			if((iter->p.ip) == p.ip){
				return &iter->p; // found it
			}
			iter = iter->next;
		}
	}
	return 0;
}




