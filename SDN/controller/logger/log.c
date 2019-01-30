/*******************************************
 * log sequence number, msg type, time, the data needed.
 * different function for different msg type
 * 
 * logStart:
 * 	Filename->controller<time>
 * RegReqm msg: 
 * 	Log #:Time:Message type:Switch ID
 * RegResp msg:
 * 	Log #:Time:Message type:Switch ID
 * 
 * 
 * *****************************************/
#include "log.h"

FILE *f;
int logEntry;
char fileName[50];
	
char *getTime(){
	time_t rawtime;
    struct tm * timeinfo;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    
    return asctime(timeinfo);
}

unsigned long getUnixTime(){
	return (unsigned long)time(NULL); 
}

void initializeLog(){
	
	char *name = "controller";
	
	snprintf(fileName, 40, "%s_%lu", name, getUnixTime());
    	
	f = fopen(fileName, "w");
	
	if (f == NULL){
		printf("Error opening file!\n");
		exit(1);
	}
		
	fprintf(f, "Log started at: %s\n", getTime());

	logEntry = 0;
	
	fclose(f);

	return;
}

void logRegRequest(int switchID){
	
	//Log #:Time:Message type:Switch ID
	logEntry++;

	f = fopen(fileName, "a");
	
	if (f == NULL){
		printf("Error opening file!\n");
		exit(1);
	}
	
	fprintf(f, "%d)%s	REGISTER_REQUEST:SwitchID-%05d\n", logEntry, getTime(), switchID);
	
	fclose(f);

	return;
}

void logRegResponse(int switchID){
	
	//Log #:Time:Message type:Switch ID
	logEntry++;

	f = fopen(fileName, "a");
	
	if (f == NULL){
		printf("Error opening file!\n");
		exit(1);
	}
	
	fprintf(f, "%d)%s	REGISTER_RESPONSE:SwitchID-%05d\n", logEntry, getTime(), switchID);
	
	fclose(f);

	return;
}

void logTopoUpdate(int switchID, int change, char *activity, int sizee){
	
	logEntry++;
	
	f = fopen(fileName, "a");
	
	if(change){
		fprintf(f, "%d)%s	TOPOLOGY_UPDATE:SwitchID-%05d: ", logEntry, getTime(), switchID);
		int i;
		
		for(i=0;i<sizee;i++)
			fprintf(f, "%c ", activity[i]);

		fprintf(f, "\n");

	}
	
	else
		fprintf(f, "%d)%s	TOPOLOGY_UPDATE:SwitchID-%05d: No update\n", logEntry, getTime(), switchID);
	
	fclose(f);


	return;
}

void logRouteUpdate(int switchID, int yes){
	
	logEntry++;
	
	f = fopen(fileName, "a");
	
	if(yes)
		fprintf(f, "%d)%s	ROUTE_UPDATE:SwitchID-%05d\n", logEntry, getTime(), switchID);
	else
		fprintf(f, "%d)%s	TOPOLOGY_UPDATE:SwitchID-%05d: No update\n", logEntry, getTime(), switchID);
	
	fclose(f);

	return;
}
/*
int main(){
		initializeLog();
		logRegRequest(4352);
		logRegResponse(352);
		
		char d[6] = {'n','n','a','a','n','a'};
		logTopoUpdate(23211, 1, d, 6);
		logTopoUpdate(11111, 0, d, 6);
		
		routeUpdate(12221, 1);
		routeUpdate(12221, 0);
	return 0;
}
*/
