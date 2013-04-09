///*
// *  network.c
// *  elevator
// *
// *  Created by Jesper Hosen & Dag Sletteboe on 23.02.13.
// *  Copyright 2013 __MyCompanyName__. All rights reserved.
// *
// */
//
//#include <stdio.h>
//#include <stdlib.h>
//#include <sys/socket.h>
//#include <arpa/inet.h>
//#include <net/if.h>
//#include <sys/ioctl.h>
//#include <ifaddrs.h>
//#include <netdb.h>
//#include "network.h"
//#include <time.h>
//#include <errno.h>
//#include <fcntl.h>
//#include "cJSON.h"
//#include "buffer_elev.h"
//#include "communication.h"
//#define TRUE  1
//#define FALSE 0
//
//
//
//
//static int listen_socket;
//static struct nw_node *root;
//
//
//void network_init(void){
//
//	const char *myip  = getlocalip();
//	struct in_addr meaddr;
//	inet_pton(AF_INET, myip, &meaddr);
//
//	nw_initlist();
//	root->p.ip = meaddr.s_addr;
//
//
//
//	int opt = TRUE;
//	struct sockaddr_in listen_addr;
//
//	listen_addr.sin_family = AF_INET;
//	listen_addr.sin_port = htons(LISTEN_PORT);
//	listen_addr.sin_addr.s_addr = htons(INADDR_ANY); // Binding local IP-address
//
//	printf("Listen socket config:\n listen_addr.sin_family = %i;\n listen_addr.sin_port = %i;\n listen_addr.sin_addr.s_addr = %i;\n",
//		   listen_addr.sin_family ,
//		   LISTEN_PORT,
//		   listen_addr.sin_addr.s_addr
//		   );
//
//	listen_socket = socket(AF_INET, SOCK_STREAM, 0);
//	if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) == -1){
//		perror("err: setsockopt\n");
//		exit(1);
//	}
//
//	if ( bind(listen_socket, (struct sockaddr *)&listen_addr, sizeof listen_addr) == -1){
//		perror("err: bind\n");
//		exit(1);
//	}
//
//
//
//	// We are ready to listen on listen_socket!
//	pthread_t listen_tcp_thread, listen_udp_thread, send_udp_broadcast_thread;
//
//	// listen for incoming tcp connections
//	if ( (pthread_create(&listen_tcp_thread, NULL, listen_tcp, (void *) NULL)) < 0){
//		perror("err:pthread_create(listen_tcp_thread)\n");
//	}
//
//	// We are ready to broadcast
//	if ( (pthread_create(&send_udp_broadcast_thread, NULL, send_udp_broadcast, (void *) NULL)) < 0){
//		perror("err:pthread_create(send_udp_broadcast_thread)\n");
//	}
//
//	// We are ready to listen for broadcast
//	if ( (pthread_create(&listen_udp_thread, NULL, listen_udp_broadcast, (void *) NULL)) < 0){
//		perror("err:pthread_create(listen_udp_thread)\n");
//	}
//
//}
//
//
//
///* \!brief Listen for TCP connections and accept incoming.
// *
// */
//void *listen_tcp(){
//
//	// Be proactive, create objects for new peer connections:
//	struct sockaddr_in peer;
//	int new_peer_socket, c;
//	c = sizeof(struct sockaddr_in);
//
//	while(1){
//		listen(listen_socket, LISTEN_BACKLOG);
//		if ( (new_peer_socket = accept(listen_socket, (struct sockaddr *)&peer, (socklen_t*)&c)) == -1 ){
//			perror("err: accept\n");
//			exit(1);
//		}
//		char * peer_ip = inet_ntoa(peer.sin_addr);
////		printf("Some Peer(ip:%s) initiated a new TCP connection to me.\n", peer_ip);
//		struct peer newpeer = peer_object(new_peer_socket, peer.sin_addr.s_addr);
//		if(!nw_find(newpeer)){
//			nw_add(newpeer);
//			assign_com_thread(newpeer);//new_peer_socket, peer_ip); //<--- change
//		}
//		else{
//			printf("Already in connected list\n");
//			nw_printlist();
//			close(new_peer_socket);
//		}
//	}
//}
//
//
//
//void* listen_udp_broadcast(){
//
//	int sock;
//	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
//	{
//		perror("socket");
//		exit(1);
//	}
//	// Bind my UDP socket to a particular port to listen for incoming UDP responses.
//
//	struct sockaddr_in saSocket;
//	memset(&saSocket, 0, sizeof(saSocket));
//	saSocket.sin_family      = AF_INET;
//	saSocket.sin_addr.s_addr = htonl(INADDR_ANY);
//	saSocket.sin_port        = htons(UDP_LISTEN_PORT);
//
//	int opt = 1;
//	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(char*)&opt,sizeof(opt));
//
//	if (bind(sock, (struct sockaddr *) &saSocket, sizeof(saSocket)) == 0)
//	{
//		printf("Socket bound\n");
//	}
//	else {
//		perror("Bind");
//	}
//	struct sockaddr_in their_addr; // connector's address information
//	int addr_len =  sizeof(their_addr);
//	char messager[50];
//
//	while(1) {
//		if (recvfrom(sock, messager, 50 , 0, (struct sockaddr *)&their_addr, &addr_len) == -1) {
//			perror("recvfrom");
//			exit(1);
//		}
//		// Check if ip is myself or already connected.
//		char * peer_ip = inet_ntoa(their_addr.sin_addr);
//		struct peer newpeer = peer_object(0, their_addr.sin_addr.s_addr);
//		if(!nw_find(newpeer)){
//			if( connect_to_peer(their_addr.sin_addr.s_addr)==-1){
//				perror("err: connect_to_peer.\n Error when trying to initate a new connection to a peer by TCP\n");
//			}
//			else{
////				printf("Successful connected to a new peer by TPC after I received an UDP broadcast.\n");
//			}
//		}
//		else{
//			NULL;
//		}
//	}
//}
//
//
//
//void* send_udp_broadcast() {
//
//	int sock;
//	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
//	{
//		perror("socket");
//		exit(1);
//	}
//
//	int opt = 1;
//	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(char*)&opt,sizeof(opt));
//	int broadcastEnable=1;
//	setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
//
//
//	struct sockaddr_in toAddr;
//	memset(&toAddr, 0, sizeof(toAddr));
//	toAddr.sin_family = AF_INET;
//	toAddr.sin_addr.s_addr = inet_addr(LAN_BROADCAST_IP);
//	toAddr.sin_port        = htons(UDP_LISTEN_PORT);
//
//	char *message = "UDP Broadcast!";
//	while(1) {
//		if (sendto(sock, message, strlen(message), 0,(struct sockaddr *) &toAddr, sizeof(toAddr))<0) {
//			perror("sendto");
//			exit(1);
//		}
//		else {
//			NULL;
//		}
//		sleep(BROADCAST_PERIOD);
//	}
//
//}
//
//
///* \!brief Communication handler
// *
// */
//void *com_handler(void * peer){
//	// The connection is established. This is the function describing what to communicate.
//	struct peer* pinf = (struct peer*) peer;
//	struct peer p;
//	p.socket = pinf->socket; // creating a local copy of the peer object
//	p.ip = pinf->ip;
//
//	struct peer * pp = nw_get(p);
//
//
//	printf("New communication handler thread created for peer connected to socket %d \n", p.socket);
//	struct msg adder = {
//			.msgtype = OPCODE_NEWPEER,
//			.from	 = p.ip,
//	};
//	handle_msg(adder, 0);
//
//	char recv_msg[MAXRECVSIZE];//[2000];
//	char send_msg[MAXRECVSIZE];//[2000];
//	int read_size;
//	struct timeval ctime, ptime, ttime;
//	gettimeofday(&ttime,0);
//
//	int flags;
//
//	/* Set non-blocking state */
//	if (-1 == (flags = fcntl(pinf->socket, F_GETFL, 0))){
//		flags = 0;
//	}
//	fcntl(p.socket, F_SETFL, flags | O_NONBLOCK);
//
//	char * string = malloc(MAXRECVSIZE);
//	char * cjsonstr = malloc(MAXRECVSIZE);
//	while(1){
//		/* Maintain connection by passing and receiving I'm alive */
//		gettimeofday(&ctime, 0);
//		if((ctime.tv_usec - ptime.tv_usec) >= UPPERIOD || (ctime.tv_usec<ptime.tv_usec)){ // check if ctime has been zeroed out. ctime<1000000
//			struct msg packet = {
//					.msgtype = OPCODE_IMALIVE,
//			};
//			char * cout  = struct_to_byte(packet);
//			send(p.socket, cout, strlen(cout), 0);//MAXRECVSIZE
//			gettimeofday(&ptime, 0);
//		}
//
//
//		/* Receive data */
//		read_size = recv(pinf->socket, recv_msg, MAXRECVSIZE, 0);
//		if(read_size <= 0){ // ERROR/RECOVERY mode
//				if(read_size == 0){
//						printf("socket: %i \n", pinf->socket);
//						perror("err:peer disconnected.\n");
//						break;
//				}
//				else if((read_size==-1) ){
//					if( (errno != EAGAIN) && (errno != EWOULDBLOCK) ) {
//						perror("Receiving failed\n");
//						break;
//					}
//					else{
//					}
//				}
//				else{
//					0;// Did not receive anything, but no error
//				}
//		}
//		else {
//			/* Receive data */
//			/**
//			 * CHECK FOR FRAGMENTS
//			 * */
////			printf("string copyyyyyyyyyy\n");
//			strcpy(string, recv_msg);
////			string = recv_msg;
//
//			int start_i = 0;
//			int end_i = cjsonendindex(string, start_i);
//
////			printf("Whilestart\n");
//			while(end_i<(strlen(string)-1)){
////				printf("Strncpy end:%i.. \n", end_i);
//				strncpy(cjsonstr, (string+start_i), (end_i-start_i+1));
//				cjsonstr[end_i-start_i+1] = '\0';
////				printf("cjsonstr: %s\n", cjsonstr);
////				printf("..Strncpy done\n");
//				struct msg packetin = byte_to_struct(cjsonstr);
//				packetin.from = p.ip;
//				handle_msg(packetin, &ttime);
//				start_i = end_i+1;
//				end_i = cjsonendindex(string, start_i);
//
//			}
////			printf("Whiledone: end_i = %i\n", end_i);
//			if(end_i!=-1){
//				strncpy(cjsonstr, (string+start_i), (end_i-start_i+1));
//				cjsonstr[end_i-start_i+1] = '\0';
////				printf("cjsonstr: %s\n", cjsonstr);
//				struct msg packetin = byte_to_struct(cjsonstr);
//				packetin.from = p.ip;
//				handle_msg(packetin, &ttime);
//			}
////			free(cjsonstr);
////			free(string);
//
//
//
////
////			int endindex = 0;
////			int startindex = 0;
////			char * cjsonstr = malloc(read_size);
////			while(endindex<=(read_size-1)){ // has at least one packet
////				startindex = endindex;
////				endindex += cjsonendindex((recv_msg+endindex));
////				strncpy(cjsonstr, recv_msg, endindex+1);
////				if(read_size!=406){
////					printf("Startindex %i, Endindex %i\n, recv_size: %i\n", startindex, endindex, read_size);
////					//printf("%s\n", cjsonstr);
////				}
////				if(startindex!=0){
////					printf("%s\n", cjsonstr);
////				}
////				struct msg packetin = byte_to_struct(cjsonstr);
////				packetin.from = p.ip;
////				handle_msg(packetin, &ttime);
////				endindex += 1;
////				if(endindex==read_size)
////					break;
////			}
//////			printf("\tReceive done\n");
////
////
////			free(cjsonstr);
//////			if(read_size!=406)
//////				printf("Received size: %i \n, cjsonendindex: %i, from: %i\n", read_size, cjsonendindex(recv_msg), p.ip);
////			memset(recv_msg,read_size, 0);
//
//		}
//
//		gettimeofday(&ctime, 0);
//		if((ctime.tv_sec-ttime.tv_sec) > TIMEOUT){
//			printf("Currtime : % i , timeout : % i\n, if(%i > %i)\n", ctime.tv_sec, ttime.tv_sec,(ctime.tv_sec-ttime.tv_sec), TIMEOUT);
//			printf("TIMEOUT ON I'M ALIVE, socket: %i\n", pinf->socket);
//			break;
//		}
//		/* Send data */
//		struct msg elem;
//		while(!cbIsEmpty(&pp->bufout)){  // Send data from buffer
//			 cbRead(&pp->bufout, &elem);
//			 if (elem.msgtype!=OPCODE_CORRUPT){
//				 char * cout  = struct_to_byte(elem);
//				 send(pinf->socket, cout, strlen(cout), 0);//MAXRECVSIZE
//				 free(cout);
////				 printf("Sent from buffer (id _%i_, to: %i)\n", elem.msgtype, pinf->ip);
//			 }
//		}
//	}
//	// Recovery mode:
//
//	nw_rm(p);
//	struct msg recovermsg = {
//		//.msgtype	= OPCODE_PEERLOSTTAKEOVER,
//		.from 		= p.ip,
//		.to			= root->p.ip
//	};
//
//	if(highest_ip() == root->p.ip){ // I have the highest ip on the network
//		printf("I have the highest ip (ip:%i), and will take over for lost peer (ip:%i)\n", root->p.ip, pinf->ip);
//		// Tell com.module that it should be the lost peers process pair
//		recovermsg.msgtype = OPCODE_PEERLOSTTAKEOVER;
//	}
//	else{
//		printf("I do not have the highest ip (ip:%i), and will not take over for lost peer (ip:%i)\n", root->p.ip, pinf->ip);
//		recovermsg.msgtype = OPCODE_PEERLOST;
//	}
//	handle_msg(recovermsg, 0);
//	//
//	close(p.socket);
//
//
//	printf("Kill com handler thread\n");
//   	pthread_exit(0);
//}
//
//
//
//int connect_to_peer(in_addr_t peer_ip){
//	int peer_socket = socket(AF_INET, SOCK_STREAM, 0);
//
//	struct sockaddr_in peer;
//	peer.sin_family			= AF_INET;
//	peer.sin_addr.s_addr	= peer_ip;
//	peer.sin_port			= htons(LISTEN_PORT); // Connect to listen port.
//	if (connect(peer_socket, (struct sockaddr *)&peer , sizeof(peer)) < 0){
//		perror("err: connect. Connecting to peer failed\n");
//		return -1;
//	}
//
//	struct peer p = peer_object(peer_socket, peer_ip);
//
//	nw_add(p);
//	assign_com_thread(p);
//
//	return 1;
//}
//
//
//
//void assign_com_thread(struct peer p){//int peer_socket, char* peer_ip){
//	// We have a connection! Now assign a communication handler thread.
//
//	struct peer *pinf = malloc(sizeof(struct peer));
//	pinf->socket 	= p.socket;
//	pinf->ip		= p.ip;
//
//
//	pthread_t com_thread;
//
//	if( pthread_create( &com_thread , NULL ,  com_handler , pinf)<0){// peer_socket_p) < 0){
//		perror("err: pthread_create\n");
//		exit(1);
//	}
//}
//
//int terminate(struct peer p){
//	if(nw_rm(p)){
//		close(p.socket);
//		return 1;
//	}
//	return 0;
//}
//
//
//
//
//
//
//char* getlocalip() {
//    struct ifaddrs *ifaddr, *ifa;
//    int family, s;
//    char host[NI_MAXHOST];
//
//    if (getifaddrs(&ifaddr) == -1)
//    {
//        perror("getifaddrs");
//        exit(EXIT_FAILURE);
//    }
//    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
//    {
//        if (ifa->ifa_addr == NULL)
//            continue;
//        s=getnameinfo(ifa->ifa_addr,sizeof(struct sockaddr_in),host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
//        if((strcmp(ifa->ifa_name,"eth0")==0)&&(ifa->ifa_addr->sa_family==AF_INET))
//        {
//            if (s != 0)
//            {
//                printf("getnameinfo() failed: %s\n", gai_strerror(s));
//                exit(EXIT_FAILURE);
//            }
//            printf("\tInterface : <%s>\n",ifa->ifa_name );
//            printf("\t  Address : <%s>\n", host);
//            freeifaddrs(ifaddr);
//            return host;
//        }
//    }
//
//    freeifaddrs(ifaddr);
//    return -1;
//}
//
//
//
///* Linked list for keeping track of connected peers
// *
// */
//
//struct peer peer_object(int socket, in_addr_t ip){
//	struct peer p;
//	p.socket = socket;
//	p.ip = ip;
//	p.active = TRUE;
//	return p;
//}
//
//void nw_initlist(){
//	root = malloc( sizeof(struct nw_node) );
//  	root->p.socket 	= 0;
//    root->p.ip 		= 0;
//    root->next 		= 0;
//    root->prev		= 0;
//}
//int nw_printlist(){
//	struct nw_node * iter;
//	iter = root;
//	int i = 0;
//	if(iter!=0){
//		while(iter!=0){
//			printf("nw_node%i, socket = %i, ip = %i\n", i, iter->p.socket, iter->p.ip);//inet_ntoa(tmp.sin_addr) ); // <-- ERROR
//			iter = iter->next;
//			i++;
//		}
//	}
//	return 1;
//}
//
//int nw_count(){
//	struct nw_node * iter;
//	iter = root;
//	int i = 0;
//	if(iter!=0){
//		while(iter!=0){
//			iter = iter->next;
//			i++;
//		}
//	}
//	return i-1; // Do not count yourself
//}
//
//int nw_add(struct peer new){
//	struct nw_node * iter, *prev;
//	iter = root;
//	if(iter!=0){
//		while(iter->next!=0){
//			iter = iter->next;
//		}
//	}
//	iter->next = malloc(sizeof(struct nw_node));
//	prev = iter;
//	iter = iter->next;
//	if(iter==0){
//		return 0; //out of memory
//	}
//	iter->p.socket 	= new.socket;
//	iter->p.ip	 	= new.ip;
//	cbInit(&iter->p.bufout, BUFFER_SIZE);
//	iter->next  	= 0;
//	iter->prev		= prev;
//	return 1; // success
//
//}
//
//int nw_rm(struct peer p){
//	struct nw_node * iter, *prev, *tmp;
//	iter = root;
//	if(iter!=0){
//		while(iter!=0){
//			if((iter->p.ip) == p.ip && (iter->p.socket)==p.socket){
//				//tmp = malloc(sizeof(struct nw_node));
//				tmp = iter;
//				iter->prev->next = iter->next;
//				if(iter->next!=0){
//					iter->next->prev = iter->prev;
//				}
//				free(tmp);
//				return 1;
//			}
//			prev = iter;
//			iter = iter->next;
//		}
//	}
//	return 0;
//}
//
//
//int nw_find(struct peer p){
//	struct nw_node * iter;
//	iter = root;
//	if(iter!=0){
//		while(iter!=0){
//			if((iter->p.ip) == p.ip){
//				return 1; // found it
//			}
//			iter = iter->next;
//		}
//	}
//	return 0;
//}
//
//in_addr_t highest_ip(){
//	struct nw_node * iter;
//	iter = root;
//	in_addr_t highest = 0;
//	while(iter!=0){
//		if((iter->p.ip) > highest){
//				highest = iter->p.ip;
//		}
//		iter = iter->next;
//	}
//	return highest;
//}
//
//int nw_activate(struct peer p){
//	struct peer * pp = nw_get(p);
//	if(pp!=0){
//		pp->active = TRUE;
//		return 1;
//	}
//	return 0;
//
//}
//
//int nw_deactivate(struct peer p){
//	struct peer * pp = nw_get(p);
//	if(pp!=0){
//		pp->active = FALSE;
//		return 1;
//	}
//	return 0;
//}
//
//struct peer * nw_get(struct peer p){
//	struct nw_node * iter;
//	iter = root;
//	if(iter!=0){
//		while(iter!=0){
//			if((iter->p.ip) == p.ip){
//				return &iter->p; // found it
//			}
//			iter = iter->next;
//		}
//	}
//	return 0;
//}
//
//
//
//
//
///* !\brief Add struct msg to out buf for all peers.
// *
// */
//
//int sendtoallpeer(struct msg package){
//	struct peer p = {
//			.ip = TOALLIP
//	};
//	return sendtopeer(package, p);
//
//}
//
//int sendtopeer(struct msg package, struct peer p){
//	struct nw_node * iter;
//	iter = root;
//	iter = iter->next;
//
//	while(iter!=0){
//		if((iter->p.ip) == p.ip || p.ip == TOALLIP){
//			struct peer * pp = nw_get(iter->p);
//			cbWrite(&pp->bufout, &package);
////			printf("Wrote to buffer (id _%i_, to: %i)\n", package.msgtype, pp->ip);
//		}
//		iter = iter->next;
//	}
//	return 1;
//}
//


/****************
 *
 *
 *
 *
 * ****/

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
#define TRUE  1
#define FALSE 0




static int listen_socket;
static struct nw_node *root;

//
//void network_init(void){
//
//	const char *myip  = getlocalip();
//	struct in_addr meaddr;
//	inet_pton(AF_INET, myip, &meaddr);
//
//	nw_initlist();
//	root->p.ip = meaddr.s_addr;
//
//	int opt = TRUE;
//	struct sockaddr_in listen_addr;
//
//	listen_addr.sin_family = AF_INET;
//	listen_addr.sin_port = htons(LISTEN_PORT);
//	listen_addr.sin_addr.s_addr = htons(INADDR_ANY); // Binding local IP-address
//
//	printf("Listen socket config:\n listen_addr.sin_family = %i;\n listen_addr.sin_port = %i;\n listen_addr.sin_addr.s_addr = %i;\n",
//		   listen_addr.sin_family ,
//		   LISTEN_PORT,
//		   listen_addr.sin_addr.s_addr
//		   );
//
//	listen_socket = socket(AF_INET, SOCK_STREAM, 0);
//	if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) == -1){
//		perror("err: setsockopt\n");
//		exit(1);
//	}
//
//	if ( bind(listen_socket, (struct sockaddr *)&listen_addr, sizeof listen_addr) == -1){
//		perror("err: bind\n");
//		exit(1);
//	}
//
//
//
//	// We are ready to listen on listen_socket!
//	pthread_t listen_tcp_thread, listen_udp_thread, send_udp_broadcast_thread;
//
//	// listen for incoming tcp connections
//	if ( (pthread_create(&listen_tcp_thread, NULL, listen_tcp, (void *) NULL)) < 0){
//		perror("err:pthread_create(listen_tcp_thread)\n");
//	}
//
//	// We are ready to broadcast
//	if ( (pthread_create(&send_udp_broadcast_thread, NULL, send_udp_broadcast, (void *) NULL)) < 0){
//		perror("err:pthread_create(send_udp_broadcast_thread)\n");
//	}
//
//	// We are ready to listen for broadcast
//	if ( (pthread_create(&listen_udp_thread, NULL, listen_udp_broadcast, (void *) NULL)) < 0){
//		perror("err:pthread_create(listen_udp_thread)\n");
//	}
//
//}

/* State machine network*/


enum nw_state_t nw_state = INIT;
enum nw_event_t nw_event = NOEVENT;
pthread_mutex_t nw_eventMutex;

struct nw_state_action_pair_t{
	enum nw_state_t nextState;
	void (*action)(void);
	int (*guard)(void);
};
#define N_NW_STATES 3
#define N_NW_EVENTS 4
#define NW_BC_TIME  3

/* \!brief Listen for TCP connections and accept incoming.
 *
 */
void *listen_tcp(){
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
		if(!nw_find(newpeer)){
			nw_add(newpeer);
			assign_com_thread(newpeer);//new_peer_socket, peer_ip); //<--- change
            nw_setevent(CONNECTION);
		}
		else{
			printf("Already in connected list\n");
			nw_printlist();
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
		if(!nw_find(newpeer)){
			if( connect_to_peer(their_addr.sin_addr.s_addr)==-1){
				perror("err: connect_to_peer.\n Error when trying to initate a new connection to a peer by TCP\n");
			}
			else{
                nw_setevent(CONNECTION);
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



/*nextState, action, guard*/
struct nw_state_action_pair_t nw_stateTable[N_NW_STATES][N_NW_EVENTS] = {
    /* state|event:	NOEVENT						TIMEOUT_BC                             	CONNECTION                              DISCONNECTION */
    /*INIT		*/ 	{{INIT,     NULL,   NULL},  {ALONE,     NULL,               NULL},	{ONLINE,	NULL,               NULL},  {INIT,      NULL,               NULL},},
    /*ALONE     */	{{ALONE,    NULL,	NULL},  {ALONE,     NULL,               NULL},	{ONLINE,    stopbroadcast_udp,  NULL},  {ALONE,     NULL,               NULL},},
    /*ONLINE	*/	{{ONLINE,   NULL,	NULL},  {ONLINE,    stopbroadcast_udp,  NULL},	{ONLINE,	NULL,               NULL},	{ALONE,     startbroadcast_udp, isalone} },
};


pthread_t listen_tcp_thread, listen_udp_thread, send_udp_broadcast_thread, timeout_thread;
struct timeval broadcasttime;

void network_init(){
    const char *myip  = getlocalip();
    struct in_addr meaddr;
    inet_pton(AF_INET, myip, &meaddr);
    nw_initlist();
    root->p.ip = meaddr.s_addr;
    pthread_mutex_init(&nw_eventMutex, 0);
    nw_state = INIT;

    startlisten_udp();
    startlisten_tcp();
    startbroadcast_udp();

    pthread_create(&timeout_thread, NULL, start_timer, (void *) NULL);


}

void nw_setevent(enum nw_event_t evnt){
	pthread_mutex_lock(&nw_eventMutex);
	nw_event = evnt;
	pthread_mutex_unlock(&nw_eventMutex);
}

enum nw_event_t nw_getevent(){
	enum nw_event_t evcopy;
	pthread_mutex_lock(&nw_eventMutex);
	evcopy = nw_event;
	pthread_mutex_unlock(&nw_eventMutex);
	return evcopy;
}
void *start_timer(){
    struct timeval testtime;
    gettimeofday(&broadcasttime, 0);
    while(1){
        gettimeofday(&testtime, 0);
        if((testtime.tv_sec - broadcasttime.tv_sec)>NW_BC_TIME){
            printf("timeout, state is: %i\n", nw_state);
            nw_setevent(TIMEOUT_BC);
            pthread_exit(0);
        }
    }
}
int isalone(){
    return (nw_count()==0);
}

void startlisten_tcp(){
    printf("Starting TCP listen\n");
    // listen for incoming tcp connections
	if ( (pthread_create(&listen_tcp_thread, NULL, listen_tcp, (void *) NULL)) < 0){
		perror("err:pthread_create(listen_tcp_thread)\n");
	}
}

void stoplisten_tcp(){
    printf("Stopping TCP listen\n");
    pthread_kill(listen_tcp_thread, 0);
}

void startlisten_udp(){
    printf("Starting UDP listen\n");
	if ( (pthread_create(&listen_udp_thread, NULL, listen_udp_broadcast, (void *) NULL)) < 0){
		perror("err:pthread_create(listen_udp_thread)\n");
	}
}

void stoplisten_udp(){
    printf("Stopping TCP listen\n");
    pthread_kill(listen_udp_thread, 0);
}

void startbroadcast_udp(){
    printf("Starting UDP broadcast\n");
	if ( (pthread_create(&send_udp_broadcast_thread, NULL, send_udp_broadcast, (void *) NULL)) < 0){
		perror("err:pthread_create(send_udp_broadcast_thread)\n");
	}
}

void stopbroadcast_udp(){
    printf("Stopping UDP broadcast\n");
    pthread_kill(send_udp_broadcast_thread, 0);
}


void nw_statemachine_handleEvent(){
	pthread_mutex_lock(&nw_eventMutex);
	struct nw_state_action_pair_t sap = nw_stateTable[nw_state][nw_event];
	nw_event = NOEVENT;
	pthread_mutex_unlock(&nw_eventMutex);
	if(sap.guard == NULL || sap.guard()){
		if(sap.action != NULL){
			sap.action();
		}
		nw_state = sap.nextState;
	}
}
void *nw_statemachine(){
    while(1){
        nw_statemachine_handleEvent();
    }
}
pthread_t network_thread;

void network(){
    network_init();
    pthread_create(&network_thread, NULL, nw_statemachine, (void *) NULL);
}

/* \!brief Communication handler
 *
 */
void *com_handler(void * peer){
	// The connection is established. This is the function describing what to communicate.
	struct peer* pinf = (struct peer*) peer;
	struct peer p;
	p.socket = pinf->socket; // creating a local copy of the peer object
	p.ip = pinf->ip;

	struct peer * pp = nw_get(p);


	printf("New communication handler thread created for peer connected to socket %d \n", p.socket);
	struct msg adder = {
			.msgtype = OPCODE_NEWPEER,
			.from	 = p.ip,
	};
	handle_msg(adder, 0);

	char recv_msg[MAXRECVSIZE];//[2000];
	char send_msg[MAXRECVSIZE];//[2000];
	int read_size;
	struct timeval ctime, ptime, ttime;
	gettimeofday(&ttime,0);

	int flags;

	/* Set non-blocking state */
	if (-1 == (flags = fcntl(pinf->socket, F_GETFL, 0))){
		flags = 0;
	}
	fcntl(p.socket, F_SETFL, flags | O_NONBLOCK);

	char * string = malloc(MAXRECVSIZE);
	char * cjsonstr = malloc(MAXRECVSIZE);
	while(1){
		/* Maintain connection by passing and receiving I'm alive */
		gettimeofday(&ctime, 0);
		if((ctime.tv_usec - ptime.tv_usec) >= UPPERIOD || (ctime.tv_usec<ptime.tv_usec)){ // check if ctime has been zeroed out. ctime<1000000
			struct msg packet = {
					.msgtype = OPCODE_IMALIVE,
			};
			char * cout  = struct_to_byte(packet);
			send(p.socket, cout, strlen(cout), 0);//MAXRECVSIZE
			gettimeofday(&ptime, 0);
		}


		/* Receive data */
		read_size = recv(pinf->socket, recv_msg, MAXRECVSIZE, 0);
		if(read_size <= 0){ // ERROR/RECOVERY mode
				if(read_size == 0){
						printf("socket: %i \n", pinf->socket);
						perror("err:peer disconnected.\n");
						break;
				}
				else if((read_size==-1) ){
					if( (errno != EAGAIN) && (errno != EWOULDBLOCK) ) {
						perror("Receiving failed\n");
						break;
					}
					else{
					}
				}
				else{
					0;// Did not receive anything, but no error
				}
		}
		else {
/* Receive data */
			/**
			 * CHECK FOR FRAGMENTS
			 * */
//			printf("string copyyyyyyyyyy\n");
			strcpy(string, recv_msg);
//			string = recv_msg;

			int start_i = 0;
			int end_i = cjsonendindex(string, start_i);

//			printf("Whilestart\n");
			while(end_i<(strlen(string)-1)){
//				printf("Strncpy end:%i.. \n", end_i);
				strncpy(cjsonstr, (string+start_i), (end_i-start_i+1));
				cjsonstr[end_i-start_i+1] = '\0';
//				printf("cjsonstr: %s\n", cjsonstr);
//				printf("..Strncpy done\n");
				struct msg packetin = byte_to_struct(cjsonstr);
				packetin.from = p.ip;
				handle_msg(packetin, &ttime);
				start_i = end_i+1;
				end_i = cjsonendindex(string, start_i);

			}
//			printf("Whiledone: end_i = %i\n", end_i);
			if(end_i!=-1){
				strncpy(cjsonstr, (string+start_i), (end_i-start_i+1));
				cjsonstr[end_i-start_i+1] = '\0';
//				printf("cjsonstr: %s\n", cjsonstr);
				struct msg packetin = byte_to_struct(cjsonstr);
				packetin.from = p.ip;
				handle_msg(packetin, &ttime);
			}
//			free(cjsonstr);
//			free(string);
		}

		gettimeofday(&ctime, 0);
		if((ctime.tv_sec-ttime.tv_sec) > TIMEOUT){
			printf("Currtime : % i , timeout : % i\n, if(%i > %i)\n", ctime.tv_sec, ttime.tv_sec,(ctime.tv_sec-ttime.tv_sec), TIMEOUT);
			printf("TIMEOUT ON I'M ALIVE, socket: %i\n", pinf->socket);
			break;
		}
		/* Send data */
		struct msg elem;
		while(!cbIsEmpty(&pp->bufout)){  // Send data from buffer
			 cbRead(&pp->bufout, &elem);
			 if (elem.msgtype!=OPCODE_CORRUPT){
				 char * cout  = struct_to_byte(elem);
				 send(pinf->socket, cout, strlen(cout), 0);//MAXRECVSIZE
				 free(cout);
			 }
		}
	}
	// Recovery mode:

	nw_rm(p);

    nw_setevent(DISCONNECTION);

	struct msg recovermsg = {
		//.msgtype	= OPCODE_PEERLOSTTAKEOVER,
		.from 		= p.ip,
		.to			= root->p.ip
	};

	if(highest_ip() == root->p.ip){ // I have the highest ip on the network
		printf("I have the highest ip (ip:%i), and will take over for lost peer (ip:%i)\n", root->p.ip, pinf->ip);
		// Tell com.module that it should be the lost peers process pair
		recovermsg.msgtype = OPCODE_PEERLOSTTAKEOVER;
	}
	else{
		printf("I do not have the highest ip (ip:%i), and will not take over for lost peer (ip:%i)\n", root->p.ip, pinf->ip);
		recovermsg.msgtype = OPCODE_PEERLOST;
	}
	handle_msg(recovermsg, 0);
	//
	close(p.socket);


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

	nw_add(p);
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

int terminate(struct peer p){
	if(nw_rm(p)){
		close(p.socket);
		return 1;
	}
	return 0;
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

struct peer peer_object(int socket, in_addr_t ip){
	struct peer p;
	p.socket = socket;
	p.ip = ip;
	p.active = TRUE;
	return p;
}

void nw_initlist(){
	root = malloc( sizeof(struct nw_node) );
  	root->p.socket 	= 0;
    root->p.ip 		= 0;
    root->next 		= 0;
    root->prev		= 0;
}
int nw_printlist(){
	struct nw_node * iter;
	iter = root;
	int i = 0;
	if(iter!=0){
		while(iter!=0){
			printf("nw_node%i, socket = %i, ip = %i\n", i, iter->p.socket, iter->p.ip);//inet_ntoa(tmp.sin_addr) ); // <-- ERROR
			iter = iter->next;
			i++;
		}
	}
	return 1;
}

int nw_count(){
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

int nw_add(struct peer new){
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

int nw_rm(struct peer p){
	struct nw_node * iter, *prev, *tmp;
	iter = root;
	if(iter!=0){
		while(iter!=0){
			if((iter->p.ip) == p.ip && (iter->p.socket)==p.socket){
				//tmp = malloc(sizeof(struct nw_node));
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


int nw_find(struct peer p){
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

in_addr_t highest_ip(){
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

int nw_activate(struct peer p){
	struct peer * pp = nw_get(p);
	if(pp!=0){
		pp->active = TRUE;
		return 1;
	}
	return 0;

}

int nw_deactivate(struct peer p){
	struct peer * pp = nw_get(p);
	if(pp!=0){
		pp->active = FALSE;
		return 1;
	}
	return 0;
}

struct peer * nw_get(struct peer p){
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





/* !\brief Add struct msg to out buf for all peers.
 *
 */

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
//			printf("Wrote to buffer (id _%i_, to: %i)\n", package.msgtype, pp->ip);
		}
		iter = iter->next;
	}
	return 1;
}
