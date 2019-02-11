#ifndef LOG_H_
#define LOG_H_

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

char *getTime();
unsigned long getUnixTime();
void initializeLog();
void logRegRequest(int switchID);

void logRegResponse(int switchID);


void logTopoUpdate(int switchID, int change, char *activity, int sizee);
void logRouteUpdate(int switchID, int yes);
	

#endif
