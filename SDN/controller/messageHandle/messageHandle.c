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
    	
    else if (active=='n')
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

void createRegResponse(int switchId, char msg[], int msgSize){

	//char msg[500];
	
	//these are dummy values, 
	//the real values must be obtained via the data structure
	//use something like a hashmap->switchID:neighbours
	
	int ids[] = {234, 35234, 3, 3};
	char *address[] = {"127.0.0.1","127.0.0.1","127.0.0.1","127.0.0.1"};
	int ports[] = {3452, 3534, 8989, 4322};
	char active[] = {'a','n','n','a'};
	
	stringForAllSwitch(msg, msgSize, 1, ids, ports, address, active);
	msg[msgSize-1]='\0';
	return;	
}
/*
int main(){
	
	
	printf("This is msg: %s\n",msg);
	
	return 0;
}

*/
