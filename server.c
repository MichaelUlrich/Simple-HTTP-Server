#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h> 	//inet_ntoa()
#include <regex.h>	//regcomp(), regexec()
enum{FALSE, TRUE};

//http://localhost:37864/index.html 				- sample URL
//Multi-thread responses 	-> new request new thread 	- Still need 4/17
//Issue displaying images, text is ok				- 4/23
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
void objectFound(int client, char * path) { 	//200 Response
				
	char buf[1024], *lineBuf;
	FILE *fd;
	size_t len = 0;
	ssize_t read;
	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	send(client, buf, strlen(buf), 0);
	
	if(strstr(path, "html") != NULL)  { 	//.html extension
		sprintf(buf, "Content-Type: text/html\r\n");
        	send(client, buf, strlen(buf), 0);
	} else if(strstr(path, "css") != NULL) { //.css extension
		sprintf(buf, "Content-Type: text/css\r\n");
                send(client, buf, strlen(buf), 0);
	} else if(strstr(path, "jpg") != NULL) {//.jpg extension
		sprintf(buf, "Content-Type: image/jpg\r\n");
                send(client, buf, strlen(buf), 0);
	} else if(strstr(path, "png") != NULL) {//.png extension
		sprintf(buf, "Content-Type: image/png\r\n");
                send(client, buf, strlen(buf), 0);
	} else if(strstr(path, "gif") != NULL) {//.gif extension
		sprintf(buf, "Content-Type: image/gif\r\n");
                send(client, buf, strlen(buf), 0);
	} else {				//All other extensions
		sprintf(buf, "Content-Type: text/plain\r\n");
                send(client, buf, strlen(buf), 0);
	} 
	sprintf(buf, "\r\n");
        send(client, buf, strlen(buf), 0);
	
	fd = fopen(path, "r");
	if(fd == NULL) {
		notFoundError(client); 		//404 Error
		return;
	}
	printf("\n"); //debug
	while((read = getline(&lineBuf, &len, fd)) != -1) {
		printf("%s", lineBuf); //debug
		sprintf(buf, lineBuf); //Issues displaying photos 4/23
		send(client, buf, strlen(buf), 0);

//		send(client, lineBuf, len, 0);
	}
//	send(client, fd, sizeof(fd), 0);	

}
char * messageParse(char * msg, int client) {
	
	int i = 0;
	//printf("%s\n", msg); //Display GET request
	char * temp = strtok(msg, "\r\n"), * token = malloc(sizeof(msg)); //token = GET / HTTP/1.1
	char **tokenArray = malloc(MSG_BUFFER);

	strcpy(token, temp);

	token = strtok(token, " ");
//	printf("%s\n", msg);
		while(temp) {
		
		//tokeniArray = realloc(tokenArray, sizeof(char*) * ++i);
		tokenArray[i] = token;

		if(tokenArray[i] == "POST") {
			badError(client);
			return 0; 					//Returns error, server will close
		}
//		printf("%i) %s\n", i, tokenArray[i]); //Debug
		token = strtok(NULL, " ");
		if(i == 1) { 						//Position of file name & path
			return tokenArray[i];
		}
		i++;	
	}
}
int main(int argc, char **argv) {

	int port = 0, sockFd = 0, recvd, connFd, sinSize = sizeof(struct sockaddr_in);
	struct sockaddr_in myAddr, clientAddr;
	char *msg = malloc(MSG_BUFFER), *fileName, *cwd = malloc(MSG_BUFFER), *path = malloc(MSG_BUFFER);
	regex_t regExp;

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
	
	//Create new thread
	//If found 				-> Return with 200 OK response
	//else if invalid (Regular Exp.) 	-> Return with 501 Error - done
	//Else if not found 			-> Return with 404 Error - 99.9% done	
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
			printf("got cwd: %s\n", cwd); //Debug
		}
		fileName = messageParse(msg, connFd);				//Parse GET request for path & file name
	
		if((strcmp(fileName,"/")) == 0) {
			printf("Default path\n");
			fileName = "/index.html";
		}
	
		strcpy(path, cwd);
		strcat(path, "/web");
		strcat(path, fileName);						//Combine working directory and path for regular expression check

		if(fileName == 0) {
			printf("Error getting filename from GET\n");
			return EXIT_FAILURE;
		} else { 	
			printf("File name: ");
			//printf("%s\n", fileName); Debug
		}
		if(regcomp(&regExp, "/[[a-zA-Z_0-9]]*.+(html|css|jpeg|png|gif))", 0)) { //Create regular expresion to compare to
			printf("regcomp error\n");
			exit(1);
		}
//		if(!(regexec(&regExp, fileName, 0, NULL, 0))) { //regexec() returns 0 if true
//			printf("%s is a vaild name\n", fileName);
	
			objectFound(connFd, path);




			
//		} else {
//			printf("%s is NOT a valid name\n", fileName);
//			badError(connFd);
//		}
		printf("PATH: %s\n", path);
		printf("Closing....\n");
		close(connFd);
		exit(EXIT_SUCCESS);			
	}	
	return EXIT_SUCCESS;
}
