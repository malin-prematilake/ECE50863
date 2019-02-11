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
#define M 5

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
int failID=0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

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
	printf("***************************\n");
	strcpy(dataLog, "***************************\n");
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
			printf("Switch %d: failID = %d\n", ID, failID);
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

				pthread_mutex_lock(&lock);
				neig[id].count = 0;
				pthread_mutex_unlock(&lock);
					
				neig[id].f = (char *)malloc(sizeof(char)*1);
				
				if(temp[0]=='a'){
					neig[id].f = "a";
					printf("Switch %d: Flag = %s\n", ID, neig[id].f);
				}
				else{
					neig[id].f =  "n";
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
					pthread_mutex_lock(&lock);
					neig[id].f = "f";
					pthread_mutex_unlock(&lock);
					printf("Switch %d : Neighbour ID %d set to failed flag\n", ID, id);
					printf("Switch %d: Flag = %s\n", ID, neig[id].f);
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
	for(i=1;i<NEIGHBSIZE;i++){
		pthread_mutex_lock(&lock);
		if(neig[i].f == "a")
			(*countA)++;
		if(neig[i].f == "n")
			(*countNA)++;
		pthread_mutex_unlock(&lock);
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
	if(failID==0)
		sprintf(temp,"%05d", countNA);
	else
		sprintf(temp,"%05d", (countNA+1));
	strcat(buffer, temp);
	
	
	for(i=1;i<NEIGHBSIZE;i++){
		pthread_mutex_lock(&lock);
		if(neig[i].f=="a"){
			sprintf(temp,"%05d", i);
			strcat(buffer, ",");
			strcat(buffer, temp);
		}
		pthread_mutex_unlock(&lock);		
	}
	for(i=1;i<NEIGHBSIZE;i++){
		pthread_mutex_lock(&lock);
		if(neig[i].f=="n"){
			sprintf(temp,"%05d", i);
			strcat(buffer, ",");
			strcat(buffer, temp);		
		}
		pthread_mutex_unlock(&lock);
	}
	for(i=1;i<NEIGHBSIZE;i++){
		pthread_mutex_lock(&lock);
		if(neig[i].f=="f"){
			sprintf(temp,"%05d", i);
			strcat(buffer, ",");
			strcat(buffer, temp);		
		}
		pthread_mutex_unlock(&lock);
	}
	strcat(buffer, "\n");
	printf("Switch %d Topology Update Buffer =%s", ID, buffer);	
}

struct threadArg{
	int sockfd;
	struct sockaddr_in contAddr;
};

struct threadArgKA{
	int sockfd;
	struct sockaddr_in contAddr;
	int neighbCount;
};

struct threadArgListen{
	int sockfd;
	struct sockaddr_in clientaddr;
	int len;
	char buffer[BUFSIZE];
};


void *MKTopologyUpdate(void *input){
	bool flag = false;
	int i, nBytes;
	char buffer[BUFSIZE];
		//M*K seconds elapsed. If A does not receive KA from B for M*K seconds, declares the link as down. Sends a TU to controller
	while(1){
		sleep(M*K);
		flag=false;
		printf("Switch %d: %d Seconds elapsed\n", ID, M*K);
		for(i=1;i<NEIGHBSIZE;i++){
			pthread_mutex_lock(&lock);
			if(neig[i].f=="a" && neig[i].count==0){
				flag=true;
				neig[i].f="n";
				//bzero((char*)&neig[i].sa, sizeof(neig[i].sa));
				printf("Switch %d: Neighbour ID %d is Unreachable. Moving A->NA\n", ID, i);
				sprintf(dataLog, "Switch %d: Neighbour ID %d is Unreachable. Moving A->NA\n", ID, i);
				LogInfo(dataLog);
			}
			neig[i].count=0;
			pthread_mutex_unlock(&lock);
		}
		if(flag){
			FormatTopologyupdate(buffer, neig);
			nBytes = sendto(((struct threadArg*)input)->sockfd, (char*)buffer, strlen(buffer), 0, (struct sockaddr*)&((struct threadArg*)input)->contAddr, sizeof(((struct threadArg*)input)->contAddr));
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

//Every K seconds send Keep Alive and Topology Update
void *KeepAliveAndTopologySender(void *input){
	char buffer[BUFSIZE];
	int i,m,nBytes;
				
	while(1){
			
			for(i=1;i<NEIGHBSIZE;i++){
				
				if((neig[i].f == "a")&&(i!=failID)){
					char alive[15];
					sprintf(alive,"K%d",ID);
					printf("Switch %d:  KEEP_ALIVE message = %s\n", ID, alive);
					nBytes = sendto(((struct threadArgKA*)input)->sockfd, (char*)alive, strlen(alive), 0, (struct sockaddr*)&neig[i].sa, sizeof(neig[i].sa));
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
			nBytes = sendto(((struct threadArgKA*)input)->sockfd, (char*)buffer, strlen(buffer), 0, (struct sockaddr*)&(((struct threadArgKA*)input)->contAddr), sizeof(((struct threadArgKA*)input)->contAddr));
			if(nBytes<0){
				printf("Switch %d: Error sending TOPOLOGY_UPDATE K to Controller\n", ID);
			}
			else{
				printf("Switch %d: Sent TOPOLOGY_UPDATE K to Controller\n", ID);
			}
			printf("Switch %d: %d Seconds Elapsed\n", ID, K);
			sleep(K);
				
	}
}

//**********
//If Switch2 receives a "KEEP ALIVE" message from Switch1, it marks Switch1 as A and learns host port information for Switch1. 
//If it receives ROUTE_UPDATE, it updates the table
//**********
void *processListener(void *input) {
	
	int recvlen, id=0, nBytes=0, pos=0, countSW, i=0; char *temp;
	//printf("Switch %d: Message Received = %s\n", ID, ((struct threadArgListen*)input)->buffer);
	char buf[BUFSIZE];
	strcpy(buf, ((struct threadArgListen*)input)->buffer);
		switch(buf[0]){
			case 'K':sscanf(&buf[1], "%d", &id);
				if(id!=failID){
					printf("Switch %d: Received KEEP ALIVE from Neighbour %d\n", ID, id);
					pthread_mutex_lock(&lock);
					neig[id].count++;
					pthread_mutex_unlock(&lock);
					if(neig[id].f=="n"){
						bzero((char*)&neig[id].sa, sizeof(neig[id].sa));
						//memcpy((void *)&clientaddr, (void *)&neig[id].sa, sizeof(clientaddr));
						/*neig[id].sa = clientaddr;
						neig[id].sa.sin_family = AF_INET;
						bcopy((char*)&clientaddr.sin_addr.s_addr, (char*)&neig[id].sa.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr));
						bcopy((char*)&clientaddr.sin_port, (char*)&neig[id].sa.sin_port, sizeof(clientaddr.sin_port));*/
						char tempAddr[INET_ADDRSTRLEN];
						inet_ntop(AF_INET, &(((struct threadArgListen*)input)->clientaddr.sin_addr), tempAddr, INET_ADDRSTRLEN);
						char *t; struct hostent *hp;
						/*ADDED BY MALIN*/t = (char *)malloc(sizeof(char)*INET_ADDRSTRLEN);
						strcpy(t,tempAddr);
						printf("Switch %d :IP Address= %s\n", ID, tempAddr);
						if((hp=gethostbyname(t))==NULL){
							printf("Switch %d: No server by that name\n", ID);
							/*ADDED BY MALIN*/free(t);			
							exit(0);
						}
						bcopy((char*)hp->h_addr_list[0], (char*)&neig[id].sa.sin_addr.s_addr, hp->h_length);
						neig[id].sa.sin_family = AF_INET; 
						neig[id].port = ntohs(((struct threadArgListen*)input)->clientaddr.sin_port);
						neig[id].sa.sin_port = htons(neig[id].port);
						printf("Switch %d: Port Num %d\n", ID, neig[id].port);
						neig[id].f="a";
						printf("Switch %d: Marked Neighbour %d as active\n", ID, id);
						printf("Switch %d: Neighbour %d is now reachable. Moving NA->A\n", ID, id);
						sprintf(dataLog, "Switch %d: Neighbour %d is now reachable. Moving NA->A\n", ID, i);
						LogInfo(dataLog);
					
						FormatTopologyupdate(buf, neig);
						nBytes = sendto(((struct threadArgKA*)input)->sockfd, (char*)buf, strlen(buf), 0, (struct sockaddr*)&((struct threadArgListen*)input)->clientaddr, sizeof(((struct threadArgListen*)input)->clientaddr));
						if(nBytes<0){
						printf("Switch %d: Error sending Keep Alive to Controller \n", ID);
						}
						printf("Switch %d: Sent TOPOLOGY_UPDATE Correction to Controller\n", ID);
						/*ADDED BY MALIN*/free(t);
					}
				}
				break;
			case 'U': printf("Switch %d: Received ROUTE_UPDATE from Controller\n", ID);
				countSW=0;
				temp = (char*)malloc(sizeof(100));
			  	memset(temp, 0, strlen(temp));
			  	pos=1;
			  	strncpy(temp, &buf[pos],5);
    			  	sscanf(temp, "%d", &countSW);
			  	pos+=5+1;
			  	printf("Switch %d: RU: Total Switches = %d pos = %d\n",  ID, countSW, pos);
			  	for(i=0;i<countSW;i++){
					  rout[i].id=(int*)malloc(sizeof(int));
					  rout[i].nextHop = (int*)malloc(sizeof(int));
					  strncpy(temp, &buf[pos],5);
	    				  sscanf(temp, "%d", rout[i].id);
					  pos+=5+1;
					  if((*rout[i].id)>=NEIGHBSIZE){
						printf("Switch %d: Error - ID Out of Range", ID);
						free(rout[i].id);
						free(rout[i].nextHop);
						return NULL;
					  }
					  strncpy(temp, &buf[pos],5);
	    				  sscanf(temp, "%d", rout[i].nextHop);
					  pos+=5+1;
				  }
				  printRoutingTable(rout, countSW);
				  free(temp);
				  break;
				  //printf("Switch %d: RU: Total Switches = %d \n",  ID, id);
			default:
				break;
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
	char regReqmessage[12];
	char temp[BUFSIZE];	//Temporary buffer to extract content
	struct sockaddr_in contAddr,switchaddr;
	struct hostent *hp;
	char *host;
	int len; //stores the size of the client structure
	int recvlen;//length of bytes received from client
	
	int neighbCount=0, nBytes, portC;
	
	
	//Thread to handle concurrency
	//pthread_t tid;
	pthread_t tid[3];
	
	ID = atoi(argv[1]); //Receive ID of Switch1
	sprintf(regReqmessage, "R%05d", ID);
	//printf("****The message: %s\n",regReqmessage);
	host=argv[2]; //Receive IP of Controller
	portC= atoi(argv[3]); //Receive Port of Controller

	if(argc==6){
		failID = atoi(argv[5]); //Receive failure neighbour of Switch
		printf("Neighbour ID whose link has failed = %d\n", failID);
		strcat(regReqmessage, ",");
		memset(temp, 0, strlen(temp));
		sprintf(temp, "%05d", failID);
		strcat(regReqmessage, temp);
		//To handle failure
		pthread_mutex_lock(&lock);
		neig[failID].f = "f";
		pthread_mutex_unlock(&lock);
		
	}
	else{
		strcat(regReqmessage, ",");
		memset(temp, 0, strlen(temp));
		sprintf(temp, "%05d", 0);
		strcat(regReqmessage, temp);
	}
	printf("****The message: %s\n",regReqmessage);
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

	//Implement a thread[0] to function Keep Alive and Topology Update every K seconds
	struct threadArgKA *Arms = (struct threadArgKA *)malloc(sizeof(struct threadArgKA));
	Arms->sockfd = sockfd;
	Arms->contAddr= contAddr;
	Arms->neighbCount = neighbCount;
	pthread_create(&tid[0], NULL, KeepAliveAndTopologySender, (void *)Arms);

	//Implement a thread[1] to function Updated Topology Update every M*K seconds
	struct threadArg *Arin = (struct threadArg *)malloc(sizeof(struct threadArg));
	Arin->sockfd = sockfd;
	Arin->contAddr= contAddr;
	pthread_create(&tid[1], NULL, MKTopologyUpdate, (void *)Arin);

	//Listen, get the message and give the message to the thread[2]
	while(1){
		memset(buffer, 0, strlen(buffer));
		recvlen=recvfrom(sockfd, (char*)buffer, BUFSIZE, 0, (struct sockaddr*)&switchaddr, &len);
		//When you receive a message give it to a thread.
		struct threadArgListen *Allen = (struct threadArgListen *)malloc(sizeof(struct threadArgListen));
		Allen->sockfd = sockfd;
		Allen->clientaddr= switchaddr;
		Allen->len = len;
		strcpy(Allen->buffer, buffer);
		pthread_create(&tid[2], NULL, processListener, (void *)Allen);		
	}
	

	
	//Join all of the threads
	pthread_join(tid[0], NULL);
	pthread_join(tid[1], NULL);
	pthread_join(tid[2], NULL);
	
	//Close the client socket
	close(sockfd);
	
	printf("Switch %d: Closed the socket\n", ID);
	return 0;
}
