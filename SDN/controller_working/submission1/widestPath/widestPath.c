/* Dijkstra's Algorithm in C */
#include "widestPath.h"

int readNumOfSwitches(char *fileName){
	
	FILE *fptr;
    char ch[10];
 
    fptr = fopen(fileName, "r");

    if (fptr == NULL){
        printf("Cannot open file \n");
        exit(1);
    }
    ch[0] = fgetc(fptr);
    
    int i=0;
    while ((ch[i] != '\n')&&(ch[i] != EOF)){
		i++;
        ch[i] = fgetc(fptr);
    }
    
    fclose(fptr);
	
	return (int)atoi(ch);
}

void readFile(const char* file_name, int **bandWidth, int **delay, int numOfSwitches){
	
	FILE* file = fopen (file_name, "r");
	int i = 0;
	
	int row=0, col=0, bw=0, dly=0;
	
	fscanf (file, "%d", &i);    
	
	while (!feof (file)){ 
	  fscanf (file, "%d", &row);
	  fscanf (file, "%d", &col);
	  fscanf (file, "%d", &bw);
	  fscanf (file, "%d", &dly);
	  
	  bandWidth[row-1][col-1] = bw;
	  delay[row-1][col-1] = dly;
	  
	  bandWidth[col-1][row-1] = bw;
	  delay[col-1][row-1] = dly;      
	}
	
	fclose (file);        
}

char *strrev(char *str)
{
      char *p1, *p2;

      if (! str || ! *str)
            return str;
      for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
      {
            *p1 ^= *p2;
            *p2 ^= *p1;
            *p1 ^= *p2;
      }
      return str;
}

int dijsktra(int cost[][N],int source,int target){
    int dist[N],prev[N],selected[N]={0},i,m,min,start,d,j;
    char path[N];
    
    for(i=1;i< N;i++)
    {
        dist[i] = IN;
        prev[i] = -1;
    }
    start = source;
    selected[start]=1;
    dist[start] = 0;
    while(selected[target] ==0)
    {
        min = IN;
        m = 0;
        for(i=1;i< N;i++)
        {
            d = dist[start] +cost[start][i];
            if(d< dist[i]&&selected[i]==0)
            {
                dist[i] = d;
                prev[i] = start;
            }
            if(min>dist[i] && selected[i]==0)
            {
                min = dist[i];
                m = i;
            }
        }
        start = m;
        selected[start] = 1;
    }
    start = target;
    j = 0;
    while(start != -1)
    {
        path[j++] = start+65;
        start = prev[start];
    }
    path[j]='\0';
    strrev(path);
    printf("%s", path);
    return dist[target];
}

/*
int main(){
    int cost[N][N],i,j,w,ch,co;
    int source, target,x,y;

    printf("\t The Shortest Path Algorithm ( DIJKSTRA'S ALGORITHM in C \n\n");

    for(i=1;i< N;i++)
    for(j=1;j< N;j++)
    cost[i][j] = IN;
    for(x=1;x< N;x++)
    {
        for(y=x+1;y< N;y++)
        {
            printf("Enter the weight of the path between nodes %d and %d: ",x,y);
            scanf("%d",&w);
            cost [x][y] = cost[y][x] = w;
        }
        printf("\n");
    }
    printf("\nEnter the source:");
    scanf("%d", &source);
    printf("\nEnter the target");
    scanf("%d", &target);
    co = dijsktra(cost,source,target);
    printf("\nThe Shortest Path: %d",co);
}
int main(){

	char *fileName = "../config.txt";
	
	int numOfSw = readNumOfSwitches(fileName);
	
	int **bandWidth = (int **)malloc(numOfSw*sizeof(int *));
	int **delay = (int **)malloc(numOfSw*sizeof(int *));
	
	int i,j;
	
	for (i=0;i<numOfSw;i++){
		bandWidth[i]=(int *)malloc(numOfSw*sizeof(int));
		delay[i]=(int *)malloc(numOfSw*sizeof(int));
	}
	
	
	for (i=0;i<numOfSw;i++){
		for (j=0;j<numOfSw;j++){
			bandWidth[i][j]=0;
			delay[i][j]=0;
		}
	}
	
	printf("Number of switches: %d\n", numOfSw);
	
	//readFile(fileName, (int **)bandWidth, (int **)delay, numOfSw);
	read_ints(fileName, (int **)bandWidth, (int **)delay, numOfSw);
	
	
	for (i=0;i<numOfSw;i++){
		for (j=0;j<numOfSw;j++){
			printf("%d ",bandWidth[i][j]);
		}
		printf("\n");
	}
	
	for (i=0;i<numOfSw;i++){
		for (j=0;j<numOfSw;j++){
			printf("%d ",delay[i][j]);
		}
		printf("\n");
	}
	return 0;
}
*/
