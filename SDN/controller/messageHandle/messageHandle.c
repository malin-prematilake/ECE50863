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
	
	//strcat(msg, nNumberArr);
	   //msg[0]='S';
   	//strcat(msg, nNumberArr);
	//strcpy(msg, msgIdentifier);
	//strcat(msg, nNumberArr);
	
	snprintf(msg, msgSize, "S%05d", numOfNs);
	
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
void createRouteUpdate(char *msg, int sWs, int destSwId[], int nxtHops[]){//sws is the total sumber of switches
//void createRouteUpdate(int sWs){

	char prefix[7];
	
	int j = snprintf(prefix, 8, "U%05d,", sWs);
	
	//printf("Th: %s\n",prefix);
	
	//msg should be long enough
	strcpy(msg, prefix);
	
	int i;
	
	for(i=0;i<sWs;i++){
		char str[12];
		snprintf(str, 13, "%05d:%05d,", destSwId[i], nxtHops[i]);
		strcat(msg, str);
	}
	
	return;
}

//Format: TSWID,#ofActiveSwitches,#ofNotActiveSwitches,ActiveSwIDs(each 5 digit)*DeactiveSWIDs...
void readTopoUpdate(char *msg, int activeNghbrs[], int deActiveNghbrs[], int *sender, int *activeNs, int *deactiveNs){
	
	int myID, numberActive, numberDeactive;
	
	char *rest;
	
	int n = sscanf(msg, "T%05d,%05d,%05d,", &myID, &numberActive, &numberDeactive);
	*sender = myID;
	*activeNs = numberActive;
	*deactiveNs = numberDeactive;
	
	rest = (char *)malloc(sizeof(char)*5*(numberActive+numberDeactive));
	n = sscanf(msg, "T%05d,%05d,%05d,%s", &myID, &numberActive, &numberDeactive, rest);

	//printf("This is Rest: %s\n", rest);
	/*
	int activeNghbrs[numberActive];
	int deActiveNghbrs[numberDeactive];
	*/

	int init_size = strlen(rest);
	char delim[] = ",";
	char *ptr = strtok(rest, delim);
	
	int i=0,j=0;
	
	while(ptr != NULL){
		if(i<numberActive){
			activeNghbrs[i] = atoi(ptr);
			i++;
		}
		else {
			deActiveNghbrs[j] = atoi(ptr);
			j++;
		}
		ptr = strtok(NULL, delim);
	}
	
	/*for(i=0;i<numberActive;i++)
		printf("%d ",activeNghbrs[i]);
	
	printf("\n");
	
	for(i=0;i<numberDeactive;i++)
		printf("%d ",deActiveNghbrs[i]);
	printf("\n");*/
	
	free(rest);

	return;
}
/*
int main(){
	
	int re[6];
	int ac,dc,sw;
	
	readTopoUpdate("T00012,00003,00002,23433,34352,34213,34343,45433", re, &sw, &ac, &dc);
	
	printf("active (%d) deactive %d\n ", ac, dc);
	
	
	return 0;
}
*/
