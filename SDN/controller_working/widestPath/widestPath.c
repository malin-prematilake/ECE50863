/* Dijkstra's Algorithm in C */
#include "widestPath.h"

//#define INFINITE 0
#define MAX 6	//Number of vertices


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

void readFile(const char* file_name, int **bandWidth, int **delay, int **bw2, int numOfSwitches){
	
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
		bw2[row-1][col-1] = bw;
		delay[row-1][col-1] = dly;

		bandWidth[col-1][row-1] = bw;
		bw2[col-1][row-1] = bw;
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
 
void dijkstraWidestPath(int **G,int n,int startnode, int dest[], int hops[]){
 
	int cost[n][n],bw[n],pred[n];
	int visited[n],count,maxBw,nextnode,i,j;
	
	//pred[] stores the predecessor of each node
	//count gives the number of nodes seen so far
	//create the cost matrix
	for(i=0;i<n;i++)
		for(j=0;j<n;j++)
			if(G[i][j]==0)
				cost[i][j]=INFINITE;
			else

				cost[i][j]=G[i][j];
	
	//initialize pred[],distance[] and visited[]
	for(i=0;i<n;i++)
	{
		bw[i]=cost[startnode][i];
		pred[i]=startnode;
		visited[i]=0;
	}
	
	bw[startnode]=0;
	visited[startnode]=1;
	count=1;
	
	while(count<n-1)
	{
		maxBw=INFINITE;
		
		//nextnode gives the node at maximum bandwidth
		for(i=0;i<n;i++)
			if(bw[i]>maxBw&&!visited[i])
			{
				maxBw=bw[i];
				nextnode=i;
			}
			
			//check if a better path exists through nextnode			
			visited[nextnode]=1;
			for(i=0;i<n;i++)
				if(!visited[i])
					if(((maxBw<cost[nextnode][i])?maxBw:cost[nextnode][i])>bw[i])
					{
						bw[i]=((maxBw<cost[nextnode][i])?maxBw:cost[nextnode][i]);
						pred[i]=nextnode;
					}
		count++;
	}

	
	int prev, hp=0;
	//print the path and distance of each node
	printf("WIDEST PATHS FOR SWITCH %d\n",startnode+1);
	
	for(i=0;i<n;i++){
		if(i!=startnode){
			printf("\nBandwidth of widest path: %d\n",bw[i]);
			printf("Path: ");
			
			dest[hp] = i;
			
			if(bw[i]!=0)
				printf("%d ",i+1);
			
			else{
				j=0;
				printf("%d ",j);
				hops[hp] = -1;
				hp++;
				continue;
			}
			
			j=i;
			do {
				prev = j;
				j=pred[j];
				printf("<-%d",j+1);
			} while(j!=startnode);
			//printf("\nStart node: %d\n",prev+1);
			hops[hp] = prev;
				
			hp++;
		}
	}
	printf("\n===============================\n");
}
/*

int main()
{
	int i,j,n,u;
	
	n=MAX;
	
	int hops[MAX-1];
	int dest[MAX-1];
	
	int G[MAX][MAX] = {{0, 100, 0, 0, 0, 80}, 
                       {100, 0, 50, 0, 180, 0}, 
                        {0, 50, 0, 50, 0, 150}, 
                        {0, 0, 50, 0, 100, 0}, 
                        {0, 180, 0, 100, 0, 0},
                        //{0, 0, 0, 0, 0, 0} 
                        {80, 0, 150, 0, 0, 0} 
                    };
	
	
	int **GX;
	
	GX = (int **)malloc(sizeof(int *)*MAX);
	
	for(i=0;i<MAX;i++)
		GX[i] = (int *)malloc(sizeof(int)*MAX);
		
	for(i=0;i<MAX;i++)
		for(j=0;j<MAX;j++)
			GX[i][j] = G[i][j];
	
	u = 0;//startnode-1
	dijkstraWidestPath(GX,n,u, dest, hops);
	
	
	for(i=0;i<MAX-1;i++)
		printf("Next hops: %d, %d\n",dest[i]+1,hops[i]+1);
		
	for(i=0;i<MAX;i++)
		free(GX[i]);
	
	free(GX);
		
	return 0;
}

*/
