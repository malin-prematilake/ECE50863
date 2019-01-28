#ifndef WIDEST_PATH_
#define WIDEST_PATH_

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#define IN 99
#define N 7

int readNumOfSwitches(char *fileName);
void readFile(const char* file_name, int **bandWidth, int **delay, int **edges, int numOfSwitches);
int dijsktra(int cost[][N],int source,int target);

#endif
