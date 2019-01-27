// Server side implementation of UDP client-server model 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <pthread.h>

#include "messageHandle/messageHandle.h"
#include "logger/log.h"
#include "widestPath/widestPath.h"

#define PORT	 		8080 
#define MAXLINE 		1024 
#define CONFIG_FILE 	"config.txt"
#define SIZE_OF_IP		30

#define M				1
#define K				5

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

//arrays for keeping switch properties
int *ports;
char *activeness;
char **addresses;
unsigned long *lastAccessTimes;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

unsigned long getControllerTime(){

	return (unsigned long)time(NULL);
}

void updateActiveness(int id, char state){
	activeness[id-1] = state;
	return;
}

void updateLastAccessTime(int id){
	lastAccessTimes[id-1] = getControllerTime();
	return;
}


//get the neighbours
int getNeighbours(int id, int *nghbs){
	
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
int addNewSwitch(int id, char *address, int port, char response[], int resSize){
	
	addresses[id-1] = address;
	ports[id-1] = port;
	activeness[id-1] = 'a';
	
	lastAccessTimes[id-1] = getControllerTime();
	
	int nghbrs[totalSwitchCount];
	
	//get the nns
	int numOfNs = getNeighbours(id, nghbrs);
	
	//create 3 arrays for the 3 parameters
	
	char **myAddresses = (char **)malloc(sizeof(char *)*numOfNs);
	
	int i,j;
	
	for (i=0;i<numOfNs;i++){
		myAddresses[i] = (char *)malloc(sizeof(char)*30);
		
	}
	
	int myPorts[numOfNs];
	char myActiveness[numOfNs];
	
	for(i=0;i<numOfNs;i++){
		myActiveness[i] = activeness[nghbrs[i]-1];
		
		if (activeness[nghbrs[i]]=='a'){
			myPorts[i] = ports[nghbrs[i]-1];
			
			//use strcpy
			for(j=0;j<30;j++){
				myAddresses[i][j] = addresses[nghbrs[i]][j];
				
			}
		}
	}
	
	printf("Neighbours of %d: ",id);
	for(i=0;i<totalSwitchCount;i++)
		printf("%d, ", nghbrs[i]);
	
	printf("\n");
	currentSwitchCount++;
	
	for(i=0;i<numOfNs;i++){
		printf("Nghbr stuff: %d %c %d\n",nghbrs[i],myActiveness[i],myPorts[i]);
	}
	//get the details of each neighbour
	createRegResponse(id, response, resSize, nghbrs, 3, myAddresses, myPorts, myActiveness);
	
	for (i=0;i<numOfNs;i++)
		free(myAddresses[i]);
		
	free(myAddresses);
	return 0;
}

void processMessageAndResponse(char msg[], char *address, int port, char response[], int responseSize){

	char type = msg[0];
	registerReq rq;
	
			
	switch(type){
		case 'R'://RegisterRequest
			rq = readRegReq(msg);
			printf("Current port and address: %d %s\n",port, address);
			addNewSwitch(rq.switchID, address, port, response, responseSize);
			response[responseSize-1] = EOF;
			break;
			
		case 'T': ;//TopologyUpdate
			//activeness is set to 'a', update lastAccessTime, update reachableMatrix
			//read packet
			int senderSw;
			int liveNeighbs[totalSwitchCount];
			int numOfLiveNeighbs = readTopoUpdate(msg, liveNeighbs, &senderSw);
			updateActiveness(senderSw, 'a');
			updateLastAccessTime(senderSw);
			printf("This is a T msg\n");
			break;
		
		default:
			printf("ERROR: Undefined message type\n");
			break;
	}
	return;
}

//this will keep the time and set the activeness[i] to 'n' if switch does not respond
void * timerThread(){
	
	//check last time the switch responded, compare it with the time now, if difference>M*K s set activeness to 'n'
	int i;
	
	unsigned long difference = (unsigned long)(M*K);
		
	while(1){
		unsigned long nowTime = getControllerTime();
		printf("Checking switch access times\n");
		for(i=0;i<totalSwitchCount;i++){
			pthread_mutex_lock(&lock);
				unsigned long temp1 = nowTime - lastAccessTimes[i];
				
				if(difference < temp1)
					activeness[i]='n';
			pthread_mutex_unlock(&lock);
		}
		//go to sleep for M*Ks
		sleep(M*K);
	}
}

int main() { 
	
	int sockfd; 
	char buffer[MAXLINE];
	
	char *test = "Hello client\n";
	 
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
	if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ) { 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	//thread init
	pthread_t tid[2];
	
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
	lastAccessTimes = (unsigned long*)malloc(sizeof(unsigned long)*totalSwitchCount);
	
	memset(activeness, 'n', totalSwitchCount * sizeof(char));
	memset(ports, 0, totalSwitchCount * sizeof(int));
	memset(lastAccessTimes, 0, totalSwitchCount * sizeof(unsigned long));

	for (i=0;i<totalSwitchCount;i++){
		addresses[i] = (char *)malloc(sizeof(char)*30);
		memset(addresses[i], '\0', 30 * sizeof(char));
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
	
	if( pthread_create(&tid[0], NULL, timerThread, NULL) != 0 )
	   printf("Failed to create thread\n");
	   
	int len, n; 

	while(1){
		printf("Waiting for switch...\n");
		n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
				MSG_WAITALL, ( struct sockaddr *) &cliaddr, 
				&len); 	
		buffer[n] = '\0'; 
	
		char tempAddr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(cliaddr.sin_addr), tempAddr, INET_ADDRSTRLEN);
		
		int tempPort = ntohs(cliaddr.sin_port);
		
		printf("IP address is: %s\n", tempAddr);
		printf("port is: %d\n", tempPort);
		
		printf("Msg: %s\n",buffer);

		int responseSize = 500;
		char response[responseSize];
		
		processMessageAndResponse(buffer, tempAddr, tempPort, response, responseSize);
		
		char *responseD = "TTTT\n";
		
		response[responseSize-1] = EOF;
		
		printf("The response: %s\n",response);
		
		//sendto(sockfd, (const char *)responseD, strlen(responseD), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len); 
		sendto(sockfd, (const char *)response, strlen(response), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len); 
				
		printf("Response message sent.\n");
		
		
		for (i=0;i<totalSwitchCount;i++){
			printf("----%d----%c---%s----%lu \n",ports[i],activeness[i],addresses[i],lastAccessTimes[i]);
		}
	}
	pthread_join(tid[0],NULL);
	return 0; 
} 
/*
 * 2 parallel processsses
 * 1. message handle
 * 2. timer
 * */
