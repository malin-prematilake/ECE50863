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
/*
int main(){
		initializeLog();
		logRegRequest(4352);
		logRegResponse(352);
	return 0;
}
*/
