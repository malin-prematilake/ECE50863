#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include <arpa/inet.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <pthread.h>


pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int totalSwitchCount = 10;

#define M	1
#define K	5

//this will keep the time and set the activeness[i] to 'n' if switch does not respond
void * timerThread(){
	
	//check last time the switch responded, compare it with the time now, if difference>M*K s set activeness to 'n'
	int i;

	//unsigned long nowTime = getControllerTime();
	unsigned long difference = (unsigned long)(M*K);

	while (1){
		for(i=0;i<totalSwitchCount;i++){
			pthread_mutex_lock(&lock);
				printf("Thhhhis is thread\n");
				/*unsigned long temp1 = nowTime - lastAccessTimes[i];
				
				if(difference < temp1)
					activeness[i]='n';*/
			pthread_mutex_unlock(&lock);
		}
		//go to sleep for M*Ks
		sleep(M*K);
	}
}

int main(){
	
	pthread_t tid[2];

	printf("This is main\n");
	if( pthread_create(&tid[0], NULL, timerThread, NULL) != 0 )
	   printf("Failed to create thread\n");

	while (1){
		printf("This is mainnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn\n");
		sleep(2);
	}

	pthread_join(tid[0],NULL);
	return 0;
}
