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

//network properties
#define PORT	 		8080 
#define MAXLINE 		1024 
#define CONFIG_FILE 	"config.txt"
#define SIZE_OF_IP		30

//timer thread properties
#define M				10
#define K				5

/*create struct for each msg type
 *create function to read each msg type (ones that are received)
 *create fucntion to create each msg type (ones that are sent)
 */
int currentSwitchCount = 0;
int totalSwitchCount = 0;
int enableRouteUpdate = 0;

int **bandWidth;
int **delay;
int **bWForCal;//0 if no edge, 1 if edge

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
int **neighbours;

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

int setLinks(){
	
	int change = 0;
	int prev;
	
	int i,j;
	
	for(i=0;i<totalSwitchCount;i++){
		for(j=0;j<totalSwitchCount;j++){
			prev = bWForCal[i][j];
			
			//if(bandWidth[i][j]!=0){
			if((activeness[i]=='a')&&(activeness[j]=='a')){
				bWForCal[i][j] = bandWidth[i][j];
			} else{
				bWForCal[i][j] = INFINITE;
			}
			//}
			if (prev!=bWForCal[i][j])
				change=1;
		}
	}
	return change;
}

void initializeLinks(){
	
	int i,j;
	for(i=0;i<totalSwitchCount;i++){
		for(j=0;j<totalSwitchCount;j++){		
			bWForCal[i][j] = INFINITE;
	
		}
	}
	return;
}

//create the switch obj and add it to an array (later add this to the map)
int addNewSwitch(int id, char *address, int port, char response[], int resSize){
	
	addresses[id-1] = address;
	ports[id-1] = port;
	activeness[id-1] = 'a';
	lastAccessTimes[id-1] = getControllerTime();
	
	int tx = setLinks();//tx is useless
	
	int nghbrs[totalSwitchCount];
	
	//get the number of ns
	int numOfNs = getNeighbours(id, nghbrs);
	
	int i,j;
	
	for(i=0;i<numOfNs;i++){
		neighbours[id-1][i]=nghbrs[i];
		
	}
	
	//create 3 arrays for the 3 parameters
	char **myAddresses = (char **)malloc(sizeof(char *)*numOfNs);
	
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
	
	currentSwitchCount++;
	
	if((!enableRouteUpdate)&&(totalSwitchCount==currentSwitchCount))
			enableRouteUpdate = 1;
		
	printf("Neighbours of %d: ",id);
	for(i=0;i<totalSwitchCount;i++)
		printf("%d, ", nghbrs[i]);
	
	printf("\n");
	
	/*
	for(i=0;i<numOfNs;i++){
		printf("Nghbr stuff: %d %c %d\n",nghbrs[i],myActiveness[i],myPorts[i]);
	}*/
	//get the details of each neighbour
	createRegResponse(id, response, resSize, nghbrs, numOfNs, myAddresses, myPorts, myActiveness);
	
	for (i=0;i<numOfNs;i++)
		free(myAddresses[i]);
		
	free(myAddresses);
	return 0;
}
int liveN(int id, int array[], int number){
	int i;
	
	for(i=0;i<number;i++){
		if(id==array[i])
			return 1;//live
	}
	return 0;//not live
}

//if both are active set edge, otherwise no
void updateActiveNeighbs(int liveNs[], int number){
	
	int i;
	
	for(i=0;i<number;i++){
		updateActiveness(liveNs[i],'a');
		updateLastAccessTime(liveNs[i]);
	}
	return;
}

void updateDeadNeighbs(int deadNs[], int number){
	
	int i;
	//printf("Thissssss: %d\n",number);
	for(i=0;i<number;i++){
		//printf("Thirrrrrr: %d\n",deadNs[i]);
		updateActiveness(deadNs[i],'n');
	}
	return;
}

int processMessageAndResponse(char msg[], char *address, int port, char response[], int responseSize){

	char type = msg[0];
	registerReq rq;
	
	switch(type){
		case 'R'://RegisterRequest
			rq = readRegReq(msg);
			logRegRequest(rq.switchID);
			
			//printf("Current port and address: %d %s\n",port, address);
			
			addNewSwitch(rq.switchID, address, port, response, responseSize);
			
			response[responseSize-1] = EOF;
			logRegResponse(rq.switchID);
			
			break;
			
		case 'T': ;//TopologyUpdate
			
			//read packet
			int senderSw, deadNs, numOfLiveNeighbs;
			int liveNeighbs[totalSwitchCount];
			int deadNeighbs[totalSwitchCount];
			
			readTopoUpdate(msg, liveNeighbs, deadNeighbs, &senderSw, &numOfLiveNeighbs, &deadNs);
			
			updateActiveness(senderSw, 'a');
			updateLastAccessTime(senderSw);
			
			//update activeness neighbours
			updateActiveNeighbs(liveNeighbs, numOfLiveNeighbs);
			
			updateDeadNeighbs(deadNeighbs, deadNs);
			
			int anyChange = setLinks();
			
			logTopoUpdate(senderSw, anyChange, activeness, totalSwitchCount);
			
			if (totalSwitchCount==currentSwitchCount){
				if ((anyChange)||(enableRouteUpdate)){
					printf("MUST SEND ROUTE UPDATE\n");
					//strncpy(response, "MUST SEND ROUTE UPDATE\n", 23);
					
					int destinations[totalSwitchCount-1];
					int nextHops[totalSwitchCount-1];
				
					//printf("WW\n");
						
					dijkstraWidestPath(bWForCal, totalSwitchCount, senderSw-1, destinations, nextHops);
					//printf("AA\n");
					
					int i;
					for (i=0;i<totalSwitchCount-1;i++){
						destinations[i]++;
						nextHops[i]++;
					}
					//printf("BB\n");
					
					createRouteUpdate(response, totalSwitchCount-1, destinations, nextHops);
					logRouteUpdate(senderSw, 1);
					//printf("CC\n");
					
					enableRouteUpdate = 0;
					
				} else {
					printf("NO UPDATE\n");
					strncpy(response, "0\n", 2);
					logRouteUpdate(senderSw, 0);
					return 1;
					
				}
			}
			printf("This is a T msg\n");
			break;	
	}
	return 0;
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
		
		if(1==setLinks())
			enableRouteUpdate=1;
			
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
		perror("Bind failed\n"); 
		exit(EXIT_FAILURE); 
	} 
	
	//thread init
	pthread_t tid[2];
	
	//start the log
	initializeLog();
	
	//read the config file
	totalSwitchCount = readNumOfSwitches(CONFIG_FILE);
	printf("Number of switches: %d\n",totalSwitchCount);

	int i,j;
	
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
	bWForCal = (int **)malloc(totalSwitchCount*sizeof(int *));
	neighbours = (int **)malloc(totalSwitchCount*sizeof(int *));
	
	for (i=0;i<totalSwitchCount;i++){
		bandWidth[i]=(int *)malloc(totalSwitchCount*sizeof(int));
		delay[i]=(int *)malloc(totalSwitchCount*sizeof(int));
		bWForCal[i]=(int *)malloc(totalSwitchCount*sizeof(int));
		neighbours[i]=(int *)malloc(totalSwitchCount*sizeof(int));
	}
	
	for (i=0;i<totalSwitchCount;i++){
		for (j=0;j<totalSwitchCount;j++){
			bandWidth[i][j]=INFINITE;
			delay[i][j]=0;
			bWForCal[i][j]=INFINITE;
			neighbours[i][j]=0;
		}
	}
	
	readFile(CONFIG_FILE, bandWidth, delay, bWForCal, totalSwitchCount);
	
	if( pthread_create(&tid[0], NULL, timerThread, NULL) != 0 ){
	   printf("Failed to create timer thread\n");
	   exit(1);
	}
	
	int len, n; 

	int firstTime = 1;//parameter for the first route update
	
	while(1){
		
		printf("********New Session********\n");
		
		if(firstTime){
			if(currentSwitchCount!=totalSwitchCount){
		
			} else {
				firstTime = 0;				
			}
		}
		
		/*printf("EDGES:\n");
		for (i=0;i<totalSwitchCount;i++){
			for (j=0;j<totalSwitchCount;j++){
					printf("%03d ",bWForCal[i][j]);
			}
			printf("\n");
		}
		printf("NEIGHBOURS:\n");
		for (i=0;i<totalSwitchCount;i++){
			for (j=0;j<totalSwitchCount;j++){
					printf("%d ",neighbours[i][j]);
			}
			printf("\n");
		}*/


		printf("Waiting for switch...\n");
		
		n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
				MSG_WAITALL, ( struct sockaddr *) &cliaddr, 
				&len); 	
		
		buffer[n] = '\0'; 
	
		char tempAddr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(cliaddr.sin_addr), tempAddr, INET_ADDRSTRLEN);
		
		int tempPort = ntohs(cliaddr.sin_port);
		
		printf("Message info:\n");
		printf("Sender IP address: %s\n", tempAddr);
		printf("Sender Port: %d\n", tempPort);
		printf("Message: %s\n",buffer);

		int responseSize = 500;
		char response[responseSize];
		
		int yy = processMessageAndResponse(buffer, tempAddr, tempPort, response, responseSize);
		
		response[responseSize-1] = EOF;
		
		if(yy!=1){
			printf("Response: %s\n",response);
			sendto(sockfd, (const char *)response, strlen(response), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
			printf("Response message sent\n"); 
		}
		
		printf("Current switch information:\n");
		printf("Ports|A|IP addr|Last available time\n");
		for (i=0;i<totalSwitchCount;i++){
			printf("%05d|%c|%s|%lu \n",ports[i],activeness[i],addresses[i],lastAccessTimes[i]);
		}
		
		//for (i=0;i<responseSize;i++){
			//response[i]=0;
		//}
		memset(response, 0, responseSize);
		
	}
	pthread_join(tid[0],NULL);
	return 0; 
} 
/*
int main() { 
	totalSwitchCount=6;
	int i,j;
	
	
	activeness = (char *)malloc(sizeof(char)*totalSwitchCount);
	addresses = (char **)malloc(sizeof(char*)*totalSwitchCount);
	delay = (int **)malloc(totalSwitchCount*sizeof(int *));
	
	
	memset(activeness, 'n', totalSwitchCount * sizeof(char));
	
	for (i=0;i<totalSwitchCount;i++){
		addresses[i] = (char *)malloc(sizeof(char)*30);
		memset(addresses[i], '\0', 30 * sizeof(char));
	}
	//initialize bw and delay matrices
	bandWidth = (int **)malloc(totalSwitchCount*sizeof(int *));
	bWForCal = (int **)malloc(totalSwitchCount*sizeof(int *));
	neighbours = (int **)malloc(totalSwitchCount*sizeof(int *));
	
	for (i=0;i<totalSwitchCount;i++){
		bandWidth[i]=(int *)malloc(totalSwitchCount*sizeof(int));
		bWForCal[i]=(int *)malloc(totalSwitchCount*sizeof(int));
		neighbours[i]=(int *)malloc(totalSwitchCount*sizeof(int));
		delay[i]=(int *)malloc(totalSwitchCount*sizeof(int));
	}
	for (i=0;i<totalSwitchCount;i++){
		for (j=0;j<totalSwitchCount;j++){
			bandWidth[i][j]=INFINITE;
			bWForCal[i][j]=INFINITE;
			delay[i][j]=0;
			neighbours[i][j]=0;
			
		}
	}
	readFile(CONFIG_FILE, bandWidth, delay, bWForCal, totalSwitchCount);
	
		
		
		activeness[0]='a';
		activeness[1]='a';
		activeness[2]='a';
		activeness[3]='a';
		activeness[4]='a';
		activeness[5]='a';
	
		int c = setLinks();
		
		printf("EDGES (%d):\n",c);
		for (i=0;i<totalSwitchCount;i++){
			for (j=0;j<totalSwitchCount;j++){
					printf("%03d ",bWForCal[i][j]);
			}
			printf("\n");
		}
		activeness[3]='n';
		
		c = setLinks();
		
		printf("EDGES (%d):\n",c);
		for (i=0;i<totalSwitchCount;i++){
			for (j=0;j<totalSwitchCount;j++){
					printf("%03d ",bWForCal[i][j]);
			}
			printf("\n");
		}
		
		activeness[3]='a';
		activeness[4]='n';
		
		c = setLinks();
		
		printf("EDGES (%d):\n",c);
		for (i=0;i<totalSwitchCount;i++){
			for (j=0;j<totalSwitchCount;j++){
					printf("%03d ",bWForCal[i][j]);
			}
			printf("\n");
		}
		
	return 0;
}*/
