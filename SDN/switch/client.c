#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>

#define portS1 8081
#define BUFSIZE 1024
#define NEIGHBSIZE 10
#define K 10
#define M 4

//Keep track of K seconds
void delay(int seconds)
{
	unsigned int retTime = time(0)+seconds;
	while(time(0)<retTime);
}

struct neighbour{
	struct sockaddr_in sa;
	int port;
	char *f;
	int count;
};
struct routingTable{
	int *id;
	int *nextHop;
};

struct neighbour neig[NEIGHBSIZE];
struct routingTable rout[NEIGHBSIZE];
int ID;	
int failID;

//Logging
FILE *filePoint;
char dataLog[50];

void LogInfo(char* text){
	filePoint = fopen("../ControllerLog.txt", "a");					
	fputs(text, filePoint);
	fclose(filePoint);
}

void printRoutingTable(struct routingTable rout[NEIGHBSIZE], int countSW){
	int i=0;
	 printf("Switch %d: Routing Table\n", ID);
	sprintf(dataLog, "Switch %d: Routing Table\n", ID);
	LogInfo(dataLog);
	for(i=0;i<countSW;i++){
		printf("ID = %d \tNextHop = %d\n", *rout[i].id, *rout[i].nextHop);
		sprintf(dataLog, "ID = %d \tNextHop = %d\n", *rout[i].id, *rout[i].nextHop);
		LogInfo(dataLog);
	}
	printf("***************************");
	strcpy(dataLog, "***************************");
	LogInfo(dataLog);
}

int RegisterResponseHandler(char buffer[BUFSIZE], char temp[BUFSIZE], int neighbCount, struct hostent *hp){
	int pos,i, len,id, countSW;
	
	switch(buffer[0]){
		//Switch handles a REGISTER_RESPONSE from controller
		case 'S':pos=1; 
			strncpy(temp, &buffer[pos],5);//Read characters 1-5 to find count of neighbours
			sscanf(temp, "%d", &neighbCount);
			printf("Switch %d: Handling REGISTER_RESPONSE message\n", ID);
			printf("Switch %d: Message type = %c and neighbCount = %d\n", ID, buffer[0],neighbCount);
			pos+=5;
			
			//For each of the neighbour parse the ID, active flag, IP and port
			for(i=0;i<(neighbCount);i++){
				
				memset(temp, 0, strlen(temp));
				strncpy(temp, &buffer[pos],5);
				sscanf(temp, "%d", &id);
				printf("Switch %d: Neighbour id = %d \n",ID, id);
				if(id>=NEIGHBSIZE){
					printf("Swicth %d: Error - ID Out of Range", ID);
					return 1;
				}
				memset(temp, 0, strlen(temp));
				strncpy(temp, &buffer[pos+=5+1],1);
				neig[id].count = 0;
				
				if(temp[0]=='a'){
					neig[id].f = "a";
					printf("Switch %d: Flag = %s\n", ID, neig[id].f);
				}
				else{
					neig[id].f = "n";
					printf("Switch %d: Flag = %s\n", ID, neig[id].f);
					pos+=2;
				}
				if(neig[id].f == "a"){//there s a host address and name that folows
					char *t;
					strcpy(temp, buffer);
					t=strtok(&temp[pos+=1+1], ",");
					printf("Switch %d :IP Address= %s\n", ID, t);
					len=strlen(t);
					bzero((char*)&neig[id].sa, sizeof(neig[id].sa));
					if((hp=gethostbyname(t))==NULL){
						printf("Switch %d: No server by that name\n", ID);
						return 1;			
					}
					bcopy((char*)hp->h_addr_list[0], (char*)&neig[id].sa.sin_addr.s_addr, hp->h_length);
					neig[id].sa.sin_family = AF_INET; 
					memset(temp, 0, strlen(temp));
					strcpy(temp, buffer);
					t=strtok(&(temp[pos+=len+1]), "*");
					len=strlen(t);
					sscanf(t, "%d", &(neig[id].port));
					neig[id].sa.sin_port = htons(neig[id].port);
					printf("Switch %d: PORT= %s\n",ID, t);
					pos+=(len+1);					
				}
				if(id==failID){
					neig[id].f == "f";
				}				
			}
			return neighbCount;
			break;	
		default:
			break;	
	}

}
void countNeighbourActNotAct(int *countA, int *countNA, struct neighbour neig[NEIGHBSIZE]){
	int i;
	for(i=0;i<NEIGHBSIZE;i++){
		if(neig[i].f == "a")
			(*countA)++;
		if(neig[i].f == "n")
			(*countNA)++;
	}
}


void FormatTopologyupdate(char buffer[BUFSIZE], struct neighbour neig[NEIGHBSIZE]){
	int i=0, pos=0; 
	int countA=0, countNA=0;
	char temp[BUFSIZE];
	memset(buffer, 0, strlen(buffer));
	buffer[pos++]= 'T';
	sprintf(temp, "%05d", ID);
	strcat(buffer, temp);
	strcat(buffer, ",");
	countNeighbourActNotAct(&countA, &countNA, neig);
	sprintf(temp,"%05d", countA);
	strcat(buffer, temp);
	strcat(buffer, ",");
	sprintf(temp,"%05d", countNA);
	strcat(buffer, temp);
	
	
	for(i=0;i<NEIGHBSIZE;i++){
		if(neig[i].f=="a"){
			sprintf(temp,"%05d", i);
			strcat(buffer, ",");
			strcat(buffer, temp);
		}		
	}
	for(i=0;i<NEIGHBSIZE;i++){
		if(neig[i].f=="n"){
			sprintf(temp,"%05d", i);
			strcat(buffer, ",");
			strcat(buffer, temp);		
		}
	}
	strcat(buffer, "\n");
	printf("Switch %d Topology Update Buffer =%s", ID, buffer);	
}

void KeepAliveAndTopologySender(int sockfd, int neighbCount, struct sockaddr_in contAddr, int ID){
	char buffer[BUFSIZE];
	int i,m,nBytes;
	bool flag = false;
		
	while(1){
		for(m=0;m<M;m++){
			for(i=0;i<NEIGHBSIZE;i++){
				
				if(neig[i].f == "a"){
					char alive[15];
					sprintf(alive,"K%d",ID);
					printf("Switch %d:  KEEP_ALIVE message = %s\n", ID, alive);
					nBytes = sendto(sockfd, (char*)alive, strlen(alive), 0, (struct sockaddr*)&neig[i].sa, sizeof(neig[i].sa));
					if(nBytes<0){
						printf("Switch %d: Error sending Keep Alive to ID %d\n", ID, i);
					}
					else{
						printf("Switch %d: Sent KEEP_ALIVE to ID %d\n", ID, i);	
					}				
				}
			}
			//Send TOPLOGY_UPDATE to the controller. The message includes a set of live neighbours of the switch
			FormatTopologyupdate(buffer, neig);
			nBytes = sendto(sockfd, (char*)buffer, strlen(buffer), 0, (struct sockaddr*)&contAddr, sizeof(contAddr));
			if(nBytes<0){
				printf("Switch %d: Error sending TOPOLOGY_UPDATE K to Controller\n", ID);
			}
			else{
				printf("Switch %d: Sent TOPOLOGY_UPDATE K to Controller\n", ID);
			}
			delay(K);
			printf("Switch %d: %d Seconds Elapsed\n", ID, K);
		}//M*K seconds elapsed. If A does not receive KA from B for M*K seconds, declares the link as down. Sends a TU to controller
		for(i=0;i<NEIGHBSIZE;i++){
			if(neig[i].f=="a" && neig[i].count==0){
				flag=true;
				neig[i].f="n";
				bzero((char*)&neig[i].sa, sizeof(neig[i].sa));
				sprintf(dataLog, "Switch %d: Neighbour ID %d is Unreachable. Moving A->NA\n", ID, i);
				LogInfo(dataLog);
			}
			neig[i].count=0;
		}
		if(flag){
			FormatTopologyupdate(buffer, neig);
			nBytes = sendto(sockfd, (char*)buffer, strlen(buffer), 0, (struct sockaddr*)&contAddr, sizeof(contAddr));
			if(nBytes<0){
				printf("Switch %d: Error sending TOPOLOGY_UPDATE M*K to Controller \n", ID);
			}
			else{
				printf("Switch %d: Sent TOPOLOGY_UPDATE M*K to Controller\n", ID);
			}
			flag=false;
		}
				
	}
}

struct threadArg{
	int sockfd;
	struct sockaddr_in contAddr;
};

//**********
//If Switch2 receives a "KEEP ALIVE" message from Switch1, it marks Switch1 as A and learns host port information for Switch1. 
//If it receives ROUTE_UPDATE, it updates the table
//**********
void *Listener(void *input) {
	//Keep listening on your PORT
	printf("Switch %d: Inside Listener\n", ID);
	char buffer[BUFSIZE];
	struct sockaddr_in clientaddr;
	int recvlen, id=0, nBytes=0, pos=0, countSW, i=0, len=0; char *temp;
	while(1){
		memset(buffer, 0, strlen(buffer));
		printf("Switch %d: Listening\n", ID);
		recvlen=recvfrom(((struct threadArg*)input)->sockfd, (char*)buffer, BUFSIZE, 0, (struct sockaddr *)&clientaddr, &len);
		printf("Message Received = %s\n", buffer);
		switch(buffer[0]){
			case 'K':sscanf(&buffer[1], "%d", &id);
				printf("Switch %d: Received KEEP ALIVE from Neighbour %d\n", ID, id);
				neig[id].count++;
				if(neig[id].f=="n"){
					bzero((char*)&neig[id].sa, sizeof(neig[id].sa));
					neig[id].sa=clientaddr;
					neig[id].port = ntohs(neig[id].sa.sin_port);
					neig[id].f="a";
					printf("Switch %d: Marked Neighbour %d as active\n", ID, id);
					sprintf(dataLog, "Switch %d: Neighbour %d is now reachable. Moving NA->A\n", ID, i);
					LogInfo(dataLog);
					
					FormatTopologyupdate(buffer, neig);
					nBytes = sendto(((struct threadArg*)input)->sockfd, (char*)buffer, strlen(buffer), 0, (struct sockaddr*)&((struct threadArg*)input)->contAddr, sizeof(((struct threadArg*)input)->contAddr));
					if(nBytes<0){
					printf("Switch %d: Error sending Keep Alive to Controller \n", ID);
					}
					printf("Switch %d: Sent TOPOLOGY_UPDATE Correction to Controller\n", ID);
				}				
				break;
			case 'U': printf("Switch %d: Received ROUTE_UPDATE from Controller\n", ID);
				countSW=0;
			  	memset(temp, 0, strlen(temp));
			  	pos=1;
			  	strncpy(temp, &buffer[pos],5);
    			  	sscanf(temp, "%d", &countSW);
			  	pos+=5+1;
			  	printf("Switch %d: RU: Total Switches = %d pos = %d\n",  ID, countSW, pos);
			  	for(i=0;i<countSW;i++){
					  rout[i].id=(int*)malloc(sizeof(int));
					  rout[i].nextHop = (int*)malloc(sizeof(int));
					  strncpy(temp, &buffer[pos],5);
	    				  sscanf(temp, "%d", rout[i].id);
					  pos+=5+1;
					  if((*rout[i].id)>=NEIGHBSIZE){
						printf("Switch %d: Error - ID Out of Range", ID);
						free(rout[i].id);
						free(rout[i].nextHop);
						return NULL;
					  }
					  strncpy(temp, &buffer[pos],5);
	    				  sscanf(temp, "%d", rout[i].nextHop);
					  pos+=5+1;
				  }
				  printRoutingTable(rout, countSW);
				  break;
				  printf("Switch %d: RU: Total Switches = %d \n",  ID, id);
			default:
				break;
		}
	}	
}

/*********Debugging Purposes********/
/*void *Dummy(void *input) {
    int nBytes;
    char *dum = "DUMMY";
    printf("Dummy: PortS1 %d\n", portS1);
    printf("Dummy: Socketfd %d\n", ((struct threadArg*)input)->sockfd);
    nBytes = sendto(((struct threadArg*)input)->sockfd, (char*)dum, strlen(dum), 0, (struct sockaddr*)&((struct threadArg*)input)->contAddr, sizeof(((struct threadArg*)input)->contAddr));
    if(nBytes<0){
	printf("Switch 1: Error sending DUMMY to Controller\n");
     }
 	
}*/

int main(int argc, char **argv){
	int sockfd;		//Socket descriptor
	char buffer[BUFSIZE];	//Buffer to receive messages
	char regReqmessage[6];
	char temp[BUFSIZE];	//Temporary buffer to extract content
	struct sockaddr_in contAddr,switchaddr;
	struct hostent *hp;
	char *host;
	int len; //stores the size of the client structure
	int recvlen;//length of bytes received from client
	
	int neighbCount=0, nBytes, portC;
	
	
	//Thread to handle concurrency
	pthread_t tid;

	
	ID = atoi(argv[1]); //Receive ID of Switch1
	sprintf(regReqmessage, "R%05d", ID);
	/******/printf("****The message: %s\n",regReqmessage);
	host=argv[2]; //Receive IP of Controller
	portC= atoi(argv[3]); //Receive Port of Controller

	if(argc==6){
		failID = atoi(argv[5]); //Receive failure neighbour of Switch
		printf("Neighbour ID whose link has failed = %d\n", failID);
		//To handle failure
		neig[failID].f = "f";
	}
	
	//Create socket
	if((sockfd = socket(AF_INET, SOCK_DGRAM,0))<0){
		printf("Switch %d: Socket creation failed\n", ID);
		return 0;
	}

	bzero((char*)&contAddr, sizeof(contAddr));
	
	//look up the address of the controller from its name
	if((hp=gethostbyname(host))==NULL){
		printf("Switch %d: No server by that name\n", ID);
		return 0;
	}
	
	//Put the host address into the controller address structure
	bcopy((char*)hp->h_addr_list[0], (char*)&contAddr.sin_addr.s_addr,hp->h_length);

	
	//Fill in the controller information
	contAddr.sin_family = AF_INET; //The address family used to set up teh socket IPV4
	contAddr.sin_port = htons(portC);//host address format into network address format short(used for int)
		
	//There is no need to bind at the client
	
	//Send REGISTER_REQUEST and Receive REGISTER_RESPONSE
	sendto(sockfd, (char*)regReqmessage, strlen(regReqmessage), 0, (struct sockaddr*)&contAddr, sizeof(contAddr));
	sprintf(dataLog, "Switch %d: Sent REGISTER_REQUEST. Message = %s\n", ID, regReqmessage);
	LogInfo(dataLog);
	recvlen=recvfrom(sockfd, (char*)buffer, BUFSIZE, 0, (struct sockaddr*)&contAddr, &len);
	if(recvlen>0){
		buffer[recvlen]='\0';
		printf("Switch %d: Received Message: %s\n", ID, buffer);
	}
	sprintf(dataLog, "Switch %d: Received REGISTER_RESPONSE. Message = %s\n", ID, buffer);
	LogInfo(dataLog);
	
	neighbCount = RegisterResponseHandler(buffer, temp, neighbCount, hp);
	printf("neighbCount  = %d\n", neighbCount);
	
	//Send KEEP_ALIVE to active neighbours every 5 seconds in the Main() thread
	//KeepAliveAndTopologySender(sockfd, neighbCount, &contAddr, ID);
	
	//DONT CLOSE //Close the client socket
	/*close(sockfd);

	//Receive a keep alive from active neighbours - Bind to your own distinct port and keep listening
	if((sockfd = socket(AF_INET, SOCK_DGRAM,0))<0){
		printf("Switch %d: Socket creation failed\n", ID);//Prints a descriptive error message to stderr
		return 0;
	}

	memset(&switchaddr,0,sizeof(switchaddr));
	memset(buffer, 0, strlen(buffer));

	switchaddr.sin_family = AF_INET; 
	if((hp=gethostbyname(host))==NULL){
		printf("Switch %d: No server by that name\n", ID);
		return 0;
	}
	bcopy((char*)hp->h_addr_list[0], (char*)&switchaddr.sin_addr.s_addr,hp->h_length);
	switchaddr.sin_port = htons(portS1);

	//Bind causes failure if trying to use the old socket. Hnece creted a new one to listen
	if(bind(sockfd, (const struct sockaddr*)&switchaddr, sizeof(switchaddr))<0){
		printf("Switch %d: Bind failed\n", ID);//Prints a descriptive error message to stderr
		return 0;
	}
	*/
	//Implement the listener as a thread to handle concurrency
	struct threadArg *Allen = (struct threadArg *)malloc(sizeof(struct threadArg));
	Allen->sockfd = sockfd;
	Allen->contAddr= contAddr;
	pthread_create(&tid, NULL, Listener, (void *)Allen);
	printf("AA\n");
	//Send KEEP_ALIVE to active neighbours every 5 seconds in the Main() thread
	KeepAliveAndTopologySender(sockfd, neighbCount, contAddr, ID);
	printf("ZZ\n");

	pthread_join(tid, NULL);
	
	//Close the client socket
	close(sockfd);
	
	printf("Switch %d: Closed the socket\n", ID);
	return 0;
}
