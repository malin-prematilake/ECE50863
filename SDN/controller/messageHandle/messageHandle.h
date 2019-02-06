#ifndef MSG_HANDLE_
#define MSG_HANDLE_

#include <stdio.h>
#include <string.h>
#include "../logger/log.h"

//struct for REGISTER_REQUEST
typedef struct{
	int switchID;
} registerReq;

//convert 5 digit char array to int
int charTo5Int(char arr[]);

//reading REGISTER_REQUEST
registerReq readRegReq(char *msg);

int intToStr(int x, char str[], int d);

void createRegResponse(int switchId, char msg[], int msgSize, int ids[], int n, char *address[], int ports[], char active[]);
void createRouteUpdate(char *msg, int sWs, int destSwId[], int nxtHops[], char activeness[]);

void readTopoUpdate(char *msg, int nghbrs[], int ff[], int *sender, int *activeNs, int *deactiveNs);
	
#endif
