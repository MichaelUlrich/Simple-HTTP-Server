#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h> //inet_ntoa()
enum{FALSE, TRUE};
//User enters port
//HTTP request
//Read html, displays in Chrome
//http://localhost:37864/index.html - sample URL
//Multi-thread responses -> new request new thread
int main(int argc, char **argv) {

	int port = 0, sockFd = 0, data, connFd, sinSize = sizeof(struct sockaddr_in);
	struct sockaddr_in myAddr, clientAddr;
	
	if(argc <= 1) { //Exit if no arguments
		printf("NO PORT ENTERED. EXITING...\n");
		exit(EXIT_FAILURE);
	} else if (argc == 2) { //Convert port value from string to int
		port = atoi(argv[1]); //char to int conversion
		printf("Port: %i\n", port);
	
	} else if (argc >= 3) { //Exit if too many arguments
		printf("TOO MANY ARGUMENTS ENTERED. EXITING...\n");
		exit(EXIT_FAILURE);
	}
	// ((sockFd = socket(AF_INET, SOCK_STREAM, 0)  == -1))	
	if ((sockFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) { //Creating TCP socket
		perror("Error creating socket\n");
		exit(EXIT_FAILURE);
	}
	
	myAddr.sin_family	= AF_INET;		//Unsigned short, Network byte order
	myAddr.sin_port   	= htons(port);		//Unsigned int, Network byte order
	myAddr.sin_addr.s_addr 	= htonl(INADDR_ANY);	//Zero struck [ bzero(&(my_addr.sin_zero), 8); ]
	if((bind(sockFd, (struct sockaddr*) &myAddr,sizeof(myAddr))) == -1) {	//Bind IP/Port to socket
		perror("Error binding socket\n");
		exit(EXIT_FAILURE);
	}
	
	listen(sockFd, 10);	//Listen on socket, time out after 10 attempts
	
	while(TRUE) {
		if((connFd = accept(sockFd, (struct sockaddr*) &clientAddr, &sinSize)) == -1) {
			perror("Error accepting connection\n");
			exit(EXIT_FAILURE);
		}		
		printf("Connected to [IP/PORT]: [%s / %d]\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
		
	}
	
	return EXIT_SUCCESS;
}
