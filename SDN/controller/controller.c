// Server side implementation of UDP client-server model 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#include "messageHandle/messageHandle.h"

#define PORT	 8080 
#define MAXLINE 1024 

/*create struct for each msg type
 *create function to read each msg type (ones that are received)
 *create fucntion to create each msg type (ones that are sent)
 */
int switchCount = 0;

typedef struct{
	int id;
	char *address;//IP address is: %s\n", inet_ntoa(cliaddr.sin_addr));
	int port;// is: %d\n", (int) ntohs(cliaddr.sin_port
	char active;
	
} switchType;

//create the switch obj and add it to an array (later add this to the map)
int addNewSwitch(int id, char *address, int port){
	
	switchType sw;
	
	sw.id = id;
	sw.address = address;
	sw.port = port;
	
	switchCount++;
	
	return 0;
}

void processMessageAndResponse(char msg[], char *address, int port, char response[]){

	char type = msg[0];
	registerReq rq;
			
	switch(type){
		case 'R':
			rq = readRegReq(msg);
			addNewSwitch(rq.switchID, address, port);
			createRegResponse(rq.switchID, response);
			break;
			
		case 'T':
			printf("THis is a T msg\n");
			break;
	}
	return;
}

int main() { 
	int sockfd; 
	char buffer[MAXLINE];
	char *hello = "Hello client\n";
	 
	struct sockaddr_in servaddr, cliaddr; 
	
	// Creating socket file descriptor 
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
		perror("socket creation failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	memset(&servaddr, 0, sizeof(servaddr)); 
	memset(&cliaddr, 0, sizeof(cliaddr)); 
	
	// Filling server information 
	servaddr.sin_family = AF_INET; // IPv4 
	servaddr.sin_addr.s_addr = INADDR_ANY; 
	servaddr.sin_port = htons(PORT); 
	
	// Bind the socket with the server address 
	if ( bind(sockfd, (const struct sockaddr *)&servaddr, 
			sizeof(servaddr)) < 0 ) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	int len, n; 
	n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
				MSG_WAITALL, ( struct sockaddr *) &cliaddr, 
				&len); 
	buffer[n] = '\0'; 
	

	/*printf("IP address is: %s\n", inet_ntoa(cliaddr.sin_addr));
	printf("port is: %d\n", (int) ntohs(cliaddr.sin_port));
	printf("Client : %s\n", buffer);*/
	
	printf("Msg: %s\n",buffer);

	char response[35];
	processMessageAndResponse(buffer, inet_ntoa(cliaddr.sin_addr), (int) ntohs(cliaddr.sin_port), response);
	
	printf("The response: %s\n",response);
	sendto(sockfd, (const char *)response, strlen(response), 
		MSG_CONFIRM, (const struct sockaddr *) &cliaddr, 
			len); 
			
	printf("Response message sent.\n"); 
	
	return 0; 
} 
