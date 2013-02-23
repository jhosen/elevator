#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define UDP_PORT 12345

void* listen_broadcast(){

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
	saSocket.sin_port        = htons(UDP_PORT);

	int opt = 1;
	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(char*)&opt,sizeof(opt));

	if (bind(sock, (struct sockaddr *) &saSocket, sizeof(saSocket)) == 0)
	{
		printf("Socket bound\n");
	}
	else {
		perror("Bind");
	}

//	struct sockaddr_in toAddr;
//	memset(&toAddr, 0, sizeof(toAddr));
//	toAddr.sin_family = AF_INET;
//	toAddr.sin_addr.s_addr = htonl();
//	toAddr.sin_port        = htons(UDP_PORT);

	struct sockaddr_in their_addr; // connector's address information
	int addr_len =  sizeof(their_addr);
	char messager[50];
//	int r;
	while(1) {
//		if(recv(sock,messager, 50 , 0) < 0) {
//			puts("recv failed");
//			}
//		puts(messager);

		if (recvfrom(sock, messager, 50 , 0, (struct sockaddr *)&their_addr, &addr_len) == -1) {
			perror("recvfrom");
			exit(1);
			}

		printf("got packet from %s\n",inet_ntoa(their_addr.sin_addr));

//		r = recvfrom(sock, messager, strlen(messager), 0, (struct sockaddr *) &fromAddr, &fromAddrLen);
//		if (r >= 0) {
//			if (optFromIP)   *optFromIP   = ntohl(fromAddr.sin_addr.s_addr);
//			if (optFromPort) *optFromPort = ntohs(fromAddr.sin_port);
	}
}

