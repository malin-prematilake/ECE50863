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
#define CONFIG_FILE 	"config1.txt"
#define SIZE_OF_IP		16

//timer thread properties
#define M				5
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
int **bWForCal;//0 if no edge, value if edge

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
int *switchFirstAccess;// set this to 1 at first access 

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int leyn;

unsigned long getControllerTime(){
	return (unsigned long)time(NULL);
}

void updateActiveness(int id, char state){
	pthread_mutex_lock(&lock);
		activeness[id-1] = state;
	pthread_mutex_unlock(&lock);
	return;
}

void updateLastAccessTime(int id){
	pthread_mutex_lock(&lock);
		lastAccessTimes[id-1] = getControllerTime();
	pthread_mutex_unlock(&lock);
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
			
			if((activeness[i]=='a')&&(activeness[j]=='a')){
				bWForCal[i][j] = bandWidth[i][j];
				bWForCal[j][i] = bandWidth[j][i];
			} else{
				bWForCal[i][j] = INFINITE;
				bWForCal[j][i] = INFINITE;
			}
			//}
			if (prev!=bWForCal[i][j])
				change=1;
		}
	}
	/*printf("++++++++++++++++++++++++++++++++++++\n");
	for(i=0;i<totalSwitchCount;i++){
		for(j=0;j<totalSwitchCount;j++){
			printf("%d ",edges[i][j]);
		}
		printf("\n");
	}
	printf("--------------------\n");
	for(i=0;i<totalSwitchCount;i++){
		for(j=0;j<totalSwitchCount;j++){
			printf("%03d ",bWForCal[i][j]);
		}
		printf("\n");
	}
	printf("++++++++++++++++++++++++++++++++++++\n");
	*/return change;
}
/*
void initializeLinks(){
	
	int i,j;
	for(i=0;i<totalSwitchCount;i++){
		for(j=0;j<totalSwitchCount;j++){		
			bWForCal[i][j] = INFINITE;
			edges[i][j] = 0;
		}
	}
	return;
}*/

//create the switch obj and add it to an array (later add this to the map)
int addNewSwitch(int id, char *address, int port, char response[], int resSize){
	
	addresses[id-1] = address;
	ports[id-1] = port;
	
	pthread_mutex_lock(&lock);
		activeness[id-1] = 'a';
		lastAccessTimes[id-1] = getControllerTime();
	pthread_mutex_unlock(&lock);
	
	//int tx = setLinks();//************************************************
	//initLinks();
	
	
	
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
		
		char temp;
		pthread_mutex_lock(&lock);
			temp = activeness[nghbrs[i]-1];
		pthread_mutex_unlock(&lock);
		
		myActiveness[i] = temp;//activeness[nghbrs[i]-1];
		
		//if (activeness[nghbrs[i]-1]=='a'){
		if (temp=='a'){
			myPorts[i] = ports[nghbrs[i]-1];
			
			//use strcpy
			/*for(j=0;j<30;j++){
				myAddresses[i][j] = addresses[nghbrs[i]][j];
				
			}*/
			strncpy (myAddresses[i], addresses[nghbrs[i]-1], 30);
		}
	}
	
	if (switchFirstAccess[id-1]==0){
		currentSwitchCount++;
		switchFirstAccess[id-1] = 1;
	}
	
	if((!enableRouteUpdate)&&(totalSwitchCount==currentSwitchCount))
			enableRouteUpdate = 1;
	
	/*	
	printf("Neighbours of %d: ",id);
	for(i=0;i<totalSwitchCount;i++)
		printf("%d, ", nghbrs[i]);
	
	printf("\n");
	*/
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
void updateActiveNeighbs(int id, int liveNs[], int number){
	
	int i;
	
	for(i=0;i<number;i++){
		updateActiveness(liveNs[i],'a');
		updateLastAccessTime(liveNs[i]);
	}
	//////
	for(i=0;i<number;i++){
		bWForCal[id-1][liveNs[i]-1] = bandWidth[id-1][liveNs[i]-1];
		bWForCal[liveNs[i]-1][id-1] = bandWidth[liveNs[i]-1][id-1];
	}
	return;
	
	
}

void updateDeadNeighbs(int id, int deadNs[], int number){
	//set active ns as well
	//set the edge to 0
	
	/*int i;
	//printf("Thissssss: %d\n",number);
	for(i=0;i<number;i++){
		//printf("Thirrrrrr: %d\n",deadNs[i]);
		updateActiveness(deadNs[i],'n');
	}*/
	int i;
	for(i=0;i<number;i++){
		bWForCal[id-1][deadNs[i]-1] = 0;
		bWForCal[deadNs[i]-1][id-1] = 0;
	}
	return;
}


void populate_sockaddr(int port, char addr[], struct sockaddr_in *dst_in4, socklen_t *addrlen) {
	int af = AF_INET;
	*addrlen = sizeof(*dst_in4);
	memset(dst_in4, 0, *addrlen);
	dst_in4->sin_family = af;
	dst_in4->sin_port = htons(port);
	inet_pton(af, addr, &dst_in4->sin_addr);
}


int sendRouteUpdate(int len){
	int uif, sockTemp;
	for(uif=0;uif<totalSwitchCount;uif++){
		
		int destinations[totalSwitchCount-1];
		int nextHops[totalSwitchCount-1];
	
		printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>WW\n");
			
		dijkstraWidestPath(bWForCal, totalSwitchCount, (uif+1)-1, destinations, nextHops);
		printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>AA\n");
		
		int i;
		for (i=0;i<totalSwitchCount-1;i++){
			destinations[i]++;
			nextHops[i]++;
		}
		char responseFF[500];
		
		createRouteUpdate(responseFF, totalSwitchCount-1, destinations, nextHops, activeness);
		logRouteUpdate((uif+1), 1);
		printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>BB\n");
		
		struct sockaddr_in swAddr;
		socklen_t addrlen;

		// Creating socket file descriptor 
		if ( (sockTemp = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
			perror("socket creation failed"); 
			exit(EXIT_FAILURE); 
		} 
		printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>CC\n");
		populate_sockaddr(ports[uif], addresses[uif], &swAddr, &addrlen);
		printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>DD\n");
		int n = sendto(sockTemp, (const char *)responseFF, strlen(responseFF), MSG_CONFIRM, (const struct sockaddr *)&swAddr, sizeof(swAddr));
		printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>EE\n");
		
		if (n<0)
			return 1;
			
		printf("SWITCH: %d -> %s\n",uif+1, responseFF);
		
		//logRouteUpdate(uif+1, 1);
			
	}	
	//enableRouteUpdate = 0;
	close(sockTemp);
	
	return 0;
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
			setLinks();
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
			updateActiveNeighbs(senderSw,liveNeighbs, numOfLiveNeighbs);
			
			updateDeadNeighbs(senderSw, deadNeighbs, deadNs);
			
			int anyChange = setLinks();//********************************************************
				
				
			if (totalSwitchCount==currentSwitchCount){	
				
				logTopoUpdate(senderSw, anyChange, activeness, totalSwitchCount);
				
				if ((anyChange)||(enableRouteUpdate)){
					printf("MUST SEND ROUTE UPDATE\n");
					
					int fd = sendRouteUpdate(leyn);
					
					if(fd)
						printf("##### ERROR IN ROUTE UPDATE ALL #####\n");
				
					
					/*-----------------------------------------------------------------------------------
					int destinations[totalSwitchCount-1];
					int nextHops[totalSwitchCount-1];
						
					dijkstraWidestPath(bWForCal, totalSwitchCount, senderSw-1, destinations, nextHops);
					
					int i;
					for (i=0;i<totalSwitchCount-1;i++){
						destinations[i]++;
						nextHops[i]++;
					}
					
					createRouteUpdate(response, totalSwitchCount-1, destinations, nextHops, activeness);
					
					printf("Route update for switch %d\n",senderSw);
					
					for (i=0;i<totalSwitchCount-1;i++)
						printf("%d -> %d\n",destinations[i], nextHops[i]);
					
					printf("--------------------------\n");
					-------------------------------------------------------------------------------------*/
					logRouteUpdate(senderSw, 1);
					
					enableRouteUpdate = 0;
					
				} else {
					printf("NO UPDATE\n");
					//logRouteUpdate(senderSw, 0);
					return 1;
				}
			}
			else {
				return 1;
				printf("Count not set yet\n");
			}
			break;	
	}
	return 0;
}

//this will keep the time and set the activeness[i] to 'n' if switch does not respond
void * timerThread(){
	
	//check last time the switch responded, compare it with the time now, if difference>M*K s set activeness to 'n'
	int i,j;
	
	unsigned long difference = (unsigned long)(M*K);
		
	while(1){
		unsigned long nowTime = getControllerTime();
		
		printf("Checking activeness at interval: ");
		for(i=0;i<totalSwitchCount;i++){
			pthread_mutex_lock(&lock);
				unsigned long temp1 = nowTime - lastAccessTimes[i];
				
				if(difference < temp1)
					activeness[i]='n';			
			pthread_mutex_unlock(&lock);
			
			printf("%c ",activeness[i]);
		}
		printf("\n");
		if(1==setLinks())//****************************************************
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
	switchFirstAccess = (int*)malloc(sizeof(int)*totalSwitchCount);
	
	memset(activeness, 'n', totalSwitchCount * sizeof(char));
	memset(ports, 0, totalSwitchCount * sizeof(int));
	memset(lastAccessTimes, 0, totalSwitchCount * sizeof(unsigned long));
	memset(switchFirstAccess, 0, totalSwitchCount * sizeof(int));

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
	/*
	for (i=0;i<totalSwitchCount;i++){
		for (j=0;j<totalSwitchCount;j++){
			
			bWForCal[i][j]=bandWidth[i][j];
			printf("***** %d\n",bWForCal[i][j]);

		}
	}*/
	
	if( pthread_create(&tid[0], NULL, timerThread, NULL) != 0 ){
	   printf("Failed to create timer thread\n");
	   exit(1);
	}
	
	int len, n; 

	int firstTime = 1;//parameter for the first route update
	
	while(1){
		
		printf("****************New Session****************\n");
		
		if(firstTime){
			if(currentSwitchCount==totalSwitchCount){
		
			//} else {
				firstTime = 0;
				int fd = sendRouteUpdate(len);
				if(fd)
					printf("##### ERROR IN ROUTE UPDATE 1 #####\n");
				/*****************************************************
				int uif, sockTemp;
				for(uif=0;uif<totalSwitchCount;uif++){
					
					int destinations[totalSwitchCount-1];
					int nextHops[totalSwitchCount-1];
				
					//printf("WW\n");
						
					dijkstraWidestPath(bWForCal, totalSwitchCount, (uif+1)-1, destinations, nextHops);
					//printf("AA\n");
					
					int i;
					for (i=0;i<totalSwitchCount-1;i++){
						destinations[i]++;
						nextHops[i]++;
					}
					char responseFF[500];
					
					createRouteUpdate(responseFF, totalSwitchCount-1, destinations, nextHops, activeness);
					logRouteUpdate((uif+1), 1);
					
					
					struct sockaddr_in swAddr;
					socklen_t addrlen;
	 
					// Creating socket file descriptor 
					if ( (sockTemp = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
						perror("socket creation failed"); 
						exit(EXIT_FAILURE); 
					} 
					
					populate_sockaddr(ports[uif], addresses[i], &swAddr, &addrlen);
					sendto(sockTemp, (const char *)responseFF, strlen(responseFF), MSG_CONFIRM, (const struct sockaddr *)&swAddr, len);
					
					printf("SWITCH: %d -> %s\n",uif+1, responseFF);
						
				}	
				enableRouteUpdate = 0;
				close(sockTemp);
				**********************************************************/
				enableRouteUpdate = 0;
			}
		}
		/*
		printf("--------EDGES-------\n");
		for (i=0;i<totalSwitchCount;i++){
			for (j=0;j<totalSwitchCount;j++){
					printf("%03d ",bWForCal[i][j]);
			}
			printf("\n");
		}
		printf("--------xxxx------\n");
		for (i=0;i<totalSwitchCount;i++){
			for (j=0;j<totalSwitchCount;j++){
					printf("%d ",edges[i][j]);
			}
			printf("\n");
		}
		printf("--------------------\n");
		*/
		//printf("NEIGHBOURS:\n");
		//for (i=0;i<totalSwitchCount;i++){
			//for (j=0;j<totalSwitchCount;j++){
				//	printf("%d ",neighbours[i][j]);
			//}
			//printf("\n");
		//}


		printf("Waiting for switch...\n");
		
		n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
				MSG_WAITALL, ( struct sockaddr *) &cliaddr, 
				&len); 	
		leyn = len;
		buffer[n] = '\0'; 
	
		char tempAddr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(cliaddr.sin_addr), tempAddr, INET_ADDRSTRLEN);
		
		int tempPort = ntohs(cliaddr.sin_port);
		
		//printf("Message info:\n");
		//printf("Sender IP address: %s\n", tempAddr);
		//printf("Sender Port: %d\n", tempPort);
		printf("Request: %s\n",buffer);

		int responseSize = 500;
		char response[responseSize];
		
		int yy = processMessageAndResponse(buffer, tempAddr, tempPort, response, responseSize);
		//printf("THIS IS YY: %d\n",yy);
		response[responseSize-1] = EOF;
		
		if(yy!=1){
			printf("Response: %s\n",response);
			sendto(sockfd, (const char *)response, strlen(response), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
			//printf("Response message sent\n"); 
		}
		printf("\n================================================\n");
		printf("1AX|Ports|A|IPaddress|Last available time\n");
		
		for (i=0;i<totalSwitchCount;i++){
			printf("%03d|%05d|%c|%s|%lu \n",switchFirstAccess[i], ports[i],activeness[i],addresses[i],lastAccessTimes[i]);
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

int main(){
	
	char acc[] = {'n','a','n','a','a','n'};
	int org[6][6] = {{0, 100, 0, 200, 0, 80},
					   {100, 0, 50, 0, 180, 0},
					   {0, 50, 0, 50, 0, 150},
			           {200, 0, 50, 0, 100, 0},
			           {0, 180, 0, 100, 0, 0},
			           {80, 0, 150, 0, 0, 0}};
			 
	int cops[6][6]  = {{0, 0, 0, 0, 0, 0},
					   {0, 0, 0, 0, 0, 0},
					   {0, 0, 0, 0, 0, 0},
			           {0, 0, 0, 0, 0, 0},
			           {0, 0, 0, 0, 0, 0},
			           {0, 0, 0, 0, 0, 0}};;
	
	int change = 0;
	int prev;
	
	int i,j;
	
	for(i=0;i<6;i++){
		for(j=0;j<6;j++){
			prev = cops[i][j];
			
			//if(bandWidth[i][j]!=0){
			if((acc[i]=='a')&&(acc[j]=='a')){
				cops[i][j] = org[i][j];
			} else{
				cops[i][j] = INFINITE;
			}
			//}
			if (prev!=cops[i][j]){
				printf("(%d, %d)",i+1,j+1);
				change=1;
			}
		}
	}
	printf("Change %d\n",change);

	for(i=0;i<6;i++){
		for(j=0;j<6;j++){
			printf("%d ",cops[i][j]);
		}
		printf("\n");
	}
	change = 0;
	
	acc[0] = 'a';
	acc[1] = 'n';
	acc[2] = 'n';
	acc[3] = 'n';
	acc[4] = 'n';
	acc[5] = 'a';
	
	for(i=0;i<6;i++){
		for(j=0;j<6;j++){
			prev = cops[i][j];
			
			//if(bandWidth[i][j]!=0){
			if((acc[i]=='a')&&(acc[j]=='a')){
				cops[i][j] = org[i][j];
			} else {
				cops[i][j] = INFINITE;
			}
			//}
			if (prev!=cops[i][j]){
				printf("(%d, %d)",i+1,j+1);
				change=1;
			}
		}
	}
	printf("Change %d\n",change);

	for(i=0;i<6;i++){
		for(j=0;j<6;j++){
			printf("%d ",cops[i][j]);
		}
		printf("\n");
	}
	
	change = 0;
	
	for(i=0;i<6;i++){
		for(j=0;j<6;j++){
			prev = cops[i][j];
			
			//if(bandWidth[i][j]!=0){
			if((acc[i]=='a')&&(acc[j]=='a')){
				cops[i][j] = org[i][j];
			} else {
				cops[i][j] = INFINITE;
			}
			//}
			if (prev!=cops[i][j]){
				printf("(%d, %d)",i+1,j+1);
				change=1;
			}
		}
	}
	printf("Change %d\n",change);

	for(i=0;i<6;i++){
		for(j=0;j<6;j++){
			printf("%d ",cops[i][j]);
		}
		printf("\n");
	}
	return 0;
}
*/
