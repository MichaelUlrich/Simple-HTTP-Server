#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h> //inet_ntoa()
enum{FALSE, TRUE};
//User enters port						- Done
//HTTP request 							- Done
//Read html, displays in Chrome 				- Error's complete
//http://localhost:37864/index.html 				- sample URL
//Multi-thread responses 	-> new request new thread 	- Still need 4/17
#define MSG_BUFFER 99999 

void badError(int client){ //501 Error - Bad URL/file

	char buf[1024];
	sprintf(buf, "HTTP/1.0 501 Method Not Implemented\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "Content-Type: text/html\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<HTML><HEAD><TITLE>501  Not Implemented\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "</TITLE></HEAD>\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<BODY><h1>501 Not Implemented</h1>\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "The server either does not recognise the request method, or it lacks the ability to fulfill the request.\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "</BODY></HTML>\r\n");
	send(client, buf, strlen(buf), 0);
}
void notFoundError(int client) { //404 Error

	char buf[1024];
        sprintf(buf, "HTTP/1.0 404 Not Found\r\n");
        send(client, buf, strlen(buf), 0);
        sprintf(buf, "Content-Type: text/html\r\n");
        send(client, buf, strlen(buf), 0);
        sprintf(buf, "\r\n");
        send(client, buf, strlen(buf), 0);
        sprintf(buf, "<HTML><HEAD><TITLE>404 Not Found\r\n");
        send(client, buf, strlen(buf), 0);
        sprintf(buf, "</TITLE></HEAD>\r\n");
        send(client, buf, strlen(buf), 0);
        sprintf(buf, "<BODY><h1>404 Not Found</h1>\r\n");
        send(client, buf, strlen(buf), 0);
        sprintf(buf, "The requested resource could not be found but may be available again in the future.\r\n");
        send(client, buf, strlen(buf), 0);
        sprintf(buf, "<div style=\"color: #eeeeee; font-size: 8pt;\"> Actually, it probably won't ever be available unless this is showing up because of a bug in your program. :(</div></HTML>\r\n");
        send(client, buf, strlen(buf), 0);

}
void objectFound(int client) { 	//200 Response
				//Display
	//Parse through GET Request
	//Get file -> "GET /" 		= Default Index
	//	   -> "GET /file.html" 	= Open this file
	
}
char * messageParse(char * msg, int client) {
	/* html or htm extension*/
	//char * chtml="Content-Type: text/html\r\n"; 
	/* css extension */ 
	//char * ccss="Content-Type: text/css\r\n"; 
	/* jpg extension */ 
	//char * cjpg="Content-Type: image/jpeg\r\n"; 
	/* png extension */ 
	//char * cpng="Content-Type: image/png\r\n"; 
	/* gif extension */ 
	//char * cgif="Content-Type: image/gif\r\n"; 
	/* Any other extensions */ 
	//char * cplain="Content-Type: text/plain\r\n"; 
	int i = 0;
	//printf("%s\n", msg); //Display GET request
	char * temp = strtok(msg, "\r\n"), * token = malloc(sizeof(msg)); //token = GET / HTTP/1.1
	char **tokenArray = NULL;

	strcpy(token, temp);

	token = strtok(token, " ");

	while(temp) {
		
		tokenArray = realloc(tokenArray, sizeof(char*) * ++i);
		tokenArray[i] = token;

		if(tokenArray[i] == "POST") {
			badError(client);
			return 0; //Returns error, server will close
		}
		//printf("%i) %s\n", i, tokenArray[i]); Debug
		token = strtok(NULL, " ");
		if(i == 2) { //Position of file name & path
			return tokenArray[i];
		}	
	}
}
int main(int argc, char **argv) {

	int port = 0, sockFd = 0, recvd, connFd, sinSize = sizeof(struct sockaddr_in);
	struct sockaddr_in myAddr, clientAddr;
	char *msg = malloc(MSG_BUFFER), *fileName, *cwd = malloc(MSG_BUFFER);

	if(argc <= 1) { 							//Exit if no arguments
		printf("NO PORT ENTERED. EXITING...\n");
		exit(EXIT_FAILURE);
	} else if (argc == 2) { 						//Convert port value from string to int
		port = atoi(argv[1]); 						//char to int conversion
		printf("Port: %i\n", port);	
	} else if (argc >= 3) { 						//Exit if too many arguments
		printf("TOO MANY ARGUMENTS ENTERED. EXITING...\n");
		exit(EXIT_FAILURE);
	}
	if ((sockFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) { 		//Creating TCP socket
		perror("Error creating socket\n");
		exit(EXIT_FAILURE);
	}
	
	myAddr.sin_family	= AF_INET;					//Unsigned short, Network byte order
	myAddr.sin_port   	= htons(port);					//Unsigned int, Network byte order
	myAddr.sin_addr.s_addr 	= htonl(INADDR_ANY);				//Zero struck [ bzero(&(my_addr.sin_zero), 8); ]
	
	if((bind(sockFd, (struct sockaddr*) &myAddr,sizeof(myAddr))) == -1) {	//Bind IP/Port to socket
		perror("Error binding socket\n");
		exit(EXIT_FAILURE);
	}
	
	listen(sockFd, 10);							//Listen on socket, time out after 10 attempts
	
	while(TRUE) {
		if((connFd = accept(sockFd, (struct sockaddr*) &clientAddr, &sinSize)) == -1) {
			perror("Error accepting connection\n");
			exit(EXIT_FAILURE);
		}		
		printf("Connected to [IP/PORT]: [%s / %d]\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
		memset(msg, 0, MSG_BUFFER);
		read(connFd, msg, MSG_BUFFER - 1);
		if(getcwd(cwd, MSG_BUFFER) == 0) {
			perror("Error getting directory\n");
			exit(EXIT_FAILURE);
		} else {
		//	printf("got cwd: %s\n", cwd); Debug
		}
		fileName = messageParse(msg, connFd);				//Parse GET request for path & file name
		if(fileName == 0) {
			printf("Error getting filename from GET\n");
			return EXIT_FAILURE;
		} else { 	
			//printf("File name: ");
			//printf("%s\n", fileName); Debug
		}	
		printf("%s\n", fileName);
		printf("%s\n", cwd);
	//	unimplementedError(connFd); 					//501 Error
	//	notFoundError(connFd);						//404 Error
	//	objectFound(connFd);						//202 Response
		printf("Closing....\n");
		close(connFd);
		exit(EXIT_SUCCESS);			
	}	
	return EXIT_SUCCESS;
}
