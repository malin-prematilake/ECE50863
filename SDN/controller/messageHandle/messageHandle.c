#include "messageHandle.h"

int charTo5Int(char arr[]){

	int res=0;
	int digits[] = {10000,
					1000,
					100,
					10,
					1};

	int i;
	for(i=0;i<5;i++)
		res =  res+(arr[i]-48)*digits[i];

	return res;
}

// reverses a string 'str' of length 'len'
void reverse(char *str, int len){
    int i=0, j=len-1, temp;
    while (i<j){

        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++; j--;
    }
}


 // Converts a given integer x to string str[].  d is the number
 // of digits required in output. If d is more than the number
 // of digits in x, then 0s are added at the beginning.
int intToStr(int x, char str[], int d){
    int i = 0;
    while (x){
        str[i++] = (x%10) + '0';
        x = x/10;
    }

    // If number of digits required is more, then
    // add 0s at the beginning
    while (i < d)
        str[i++] = '0';

    reverse(str, i);
    str[i] = '\0';
    return i;
}


//reading REGISTER_REQUEST
registerReq readRegReq(char *msg){
	
	char idString[5];
	idString[0] = msg[1];
	idString[1] = msg[2];
	idString[2] = msg[3];
	idString[3] = msg[4];
	idString[4] = msg[5];
	
	int id = charTo5Int(idString);
	
	registerReq rq;
	rq.switchID = id;
	
	return rq;
	
}


void stringForOneSwitch(char msg[], int msgSize, int id, char *address, int port, char active){

	if (active=='a')
		snprintf(msg, msgSize, "%05d,%c,%s,%05d*", id,active,address,port);
    	
    else 
		snprintf(msg, msgSize, "%05d,%c*", id,active);
    
	return;
}

//create 3 arrays 
// one for ports
// one for addresses
// one for activeness
void stringForAllSwitch(char msg[], int msgSize, int numOfNs, int ids[], int ports[], char *address[], char active[]){

	int i;
	
	char msgIdentifier[] = {'S'};
	
	char nNumberArr[5];
	
	intToStr(numOfNs, nNumberArr, 5);
	
	strcpy(msg, msgIdentifier);
	//strcat(msg, nNumberArr);
	
	for(i=0;i<numOfNs;i++){
		char msgT[30];
		stringForOneSwitch(msgT, 30, ids[i], address[i], ports[i], active[i]);
		
		strcat(msg, msgT);
	}
	
	return;
}

void createRegResponse(int switchId, char msg[], int msgSize, int ids[], int numOfNs, char *address[], int ports[], char active[]){
	stringForAllSwitch(msg, msgSize, numOfNs, ids, ports, address, active);
	msg[msgSize-1]='\0';
	return;	
}

//U#TotalSwitches,sw1:nxtHop,sw2:nxtHop,...
void createRouteUpdate(char *msg, int sWs, int nxtHops[]){
//void createRouteUpdate(int sWs){

	char prefix[7];
	
	int j = snprintf(prefix, 8, "U%05d,", sWs);
	
	printf("Th: %s\n",prefix);
	
	//msg should be long enough
	strcpy(msg, prefix);
	
	int i;
	
	for(i=0;i<sWs;i++){
		char str[12];
		snprintf(str, 13, "%05d:%05d,", (i+1), nxtHops[i]);
		strcat(msg, str);
	}
	
	return;
}

//Format: TSWID,#ofSwitches,SwIDs(each 5 digit)
int readTopoUpdate(char *msg, int nghbrs[], int *sender){
	
	int myID, number;
	
	char *rest;
	
	int n = sscanf(msg, "T%05d,%05d", &myID, &number);
	*sender = myID;
	
	rest = (char *)malloc(sizeof(char)*5*number);
	n = sscanf(msg, "T%05d,%05d,%s", &myID, &number, rest);

	//printf("This is Rest: %s\n", rest);
	
	//int nghbrs[number];
	
	int init_size = strlen(rest);
	char delim[] = ",";

	char *ptr = strtok(rest, delim);

	int i=0,j;
	
	while(ptr != NULL){
		nghbrs[i] = atoi(ptr);
		i++;
		ptr = strtok(NULL, delim);
	}
	
	free(rest);
	return number;
}
/*
int main(){
	
	int re[6];
	int sw;
	
	int yy = readTopoUpdate("T00012,00003,23433,34352,34213", re, &sw);
	
	//printf("The neighbs (%d) of switch %d: ", yy, sw);
	
	int j;
	
	//for (j=0;j<yy;j++)
		//printf("%d, ",re[j]);
	//printf("\n");
	
	//U#TotalSwitches,sw1:nxtHop,sw2:nxtHop,...
	char msg[7+12*3];
	
	int df[] = {3,7,3};
	createRouteUpdate(msg, 3, df);
	
	return 0;
}
*/
