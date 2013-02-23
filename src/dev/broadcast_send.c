#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define HELLO_PORT 12345


int main(int argc, char *argv[]) {

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
	saSocket.sin_port        = htons(HELLO_PORT);

	int opt = 1;
	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(char*)&opt,sizeof(opt));
	int broadcastEnable=1;
	int ret=setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));

	if (bind(sock, (struct sockaddr *) &saSocket, sizeof(saSocket)) == 0)
	{
		printf("Socket bound\n");
	}
	else {
		perror("Bind");
	}

	struct sockaddr_in toAddr;
	memset(&toAddr, 0, sizeof(toAddr));
	toAddr.sin_family = AF_INET;
//	toAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	toAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	toAddr.sin_port        = htons(HELLO_PORT);

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
