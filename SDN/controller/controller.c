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
#include "logger/log.h"
#include "widestPath/widestPath.h"

#define PORT	 		8080 
#define MAXLINE 		1024 
#define CONFIG_FILE 	"config.txt"
#define SIZE_OF_IP		30

/*create struct for each msg type
 *create function to read each msg type (ones that are received)
 *create fucntion to create each msg type (ones that are sent)
 */
int currentSwitchCount = 0;
int totalSwitchCount = 0;

int **bandWidth;
int **delay;

typedef struct{
	int id;
	char *address;//IP address is: %s\n", inet_ntoa(cliaddr.sin_addr));
	int port;// is: %d\n", (int) ntohs(cliaddr.sin_port
	char active;
	char *neighbours;
	
} switchType;

//array for keeping all switches
switchType *swArray;
int *ports;
char *activeness;
char **addresses;

//get the neighbours
int getNeighbours(int id, char *nghbs){
	
	int i;
	int nn=0;
	
	for(i=0;i<totalSwitchCount;i++){
		nghbs[i] = 0;
		if(bandWidth[id-1][i] != 0){
			nghbs[nn] = i+1;
			nn++;
		}
	}
	return nn;
}
//create the switch obj and add it to an array (later add this to the map)
int addNewSwitch(int id, char *address, int port){
	
	addresses[id-1] = address;
	ports[id-1] = port;
	activeness[id-1] = 'a';
	
	char nghbrs[totalSwitchCount];
	
	//get the nns
	int numOfNs = getNeighbours(id, nghbrs);
	
	//create 3 arrays for the 3 parameters
	char myAddresses[numOfNs][30];
	int myPorts[numOfNs];
	char myActiveness[numOfNs];
	
	int i,j;
	
	for(i=0;i<numOfNs;i++){
		myActiveness[i] = activeness[nghbrs[i]];
		
		if (activeness[nghbrs[i]]=='a'){
			myPorts[i] = ports[nghbrs[i]];
		
			for(j=0;j<30;j++){
				myAddresses[i][j] = addresses[nghbrs[i]][j];
			}
		}
	}
	/*int i;
	for(i=0;i<totalSwitchCount;i++)
		printf("==%d\n", nghbrs[i]);
	*/
	currentSwitchCount++;
	
	//get the details of each neighbour
	
	return 0;
}

void processMessageAndResponse(char msg[], char *address, int port, char response[], int responseSize){

	char type = msg[0];
	registerReq rq;
			
	switch(type){
		case 'R':
			rq = readRegReq(msg);
			printf("*****Current: %d %s\n",port, address);
			addNewSwitch(rq.switchID, address, port);
			createRegResponse(rq.switchID, response, responseSize);
			response[responseSize-1] = EOF;
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
	
	//start the log
	initializeLog();
	
	//read the config file
	totalSwitchCount = readNumOfSwitches(CONFIG_FILE);
	printf("NUMBER OF SW: %d\n",totalSwitchCount);

	int i,j;
	
	//initialize swArray
	//swArray = (switchType *)malloc(sizeof(switchType)*totalSwitchCount);
	
	ports = (int *)malloc(sizeof(int)*totalSwitchCount);
	activeness = (char *)malloc(sizeof(char)*totalSwitchCount);
	addresses = (char **)malloc(sizeof(char*)*totalSwitchCount);
	
	for (i=0;i<totalSwitchCount;i++){
		addresses[i] = (char *)malloc(sizeof(char)*30);
		//activeness[i] = 'n';
	}
		
	//initialize bw and delay matrices
	bandWidth = (int **)malloc(totalSwitchCount*sizeof(int *));
	delay = (int **)malloc(totalSwitchCount*sizeof(int *));
	
	for (i=0;i<totalSwitchCount;i++){
		bandWidth[i]=(int *)malloc(totalSwitchCount*sizeof(int));
		delay[i]=(int *)malloc(totalSwitchCount*sizeof(int));
	}
	
	for (i=0;i<totalSwitchCount;i++){
		for (j=0;j<totalSwitchCount;j++){
			bandWidth[i][j]=0;
			delay[i][j]=0;
		}
	}
	
	readFile(CONFIG_FILE, bandWidth, delay, totalSwitchCount);
	/*
	printf("Topology bandwidths:\n");
	
	for (i=0;i<totalSwitchCount;i++){
		for (j=0;j<totalSwitchCount;j++){
			printf("%d ",bandWidth[i][j]);
		}
		printf("\n");
	}
	
	printf("Topology delays:\n");
	
	for (i=0;i<totalSwitchCount;i++){
		for (j=0;j<totalSwitchCount;j++){
			printf("%d ",delay[i][j]);
		}
		printf("\n");
	}	
*/

	int len, n; 

	while(1){
		printf("Waiting for client...\n");
		n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
				MSG_WAITALL, ( struct sockaddr *) &cliaddr, 
				&len); 
		buffer[n] = '\0'; 
	

		printf("IP address is: %s\n", inet_ntoa(cliaddr.sin_addr));
		printf("port is: %d\n", (int) ntohs(cliaddr.sin_port));
		printf("Client : %s\n", buffer);
		
		printf("Msg: %s\n",buffer);

		int responseSize = 500;
		char response[responseSize];
		
		processMessageAndResponse(buffer, inet_ntoa(cliaddr.sin_addr), (int) ntohs(cliaddr.sin_port), response, responseSize);
		
		printf("The response: %s\n",response);
		
		sendto(sockfd, (const char *)response, strlen(response), 
			MSG_CONFIRM, (const struct sockaddr *) &cliaddr, 
				len); 
				
		printf("Response message sent.\n");
		
		//for (i=0;i<totalSwitchCount;i++){
			//printf("----%d----%c---%s \n",ports[i],activeness[i],addresses[i]);
		//}
	}
	
	return 0; 
} 
