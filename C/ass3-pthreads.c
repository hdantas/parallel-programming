#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_THREADS 10
#define NELEMS(x)  (sizeof(x) / sizeof(x[0])) // number of elements in an array
#define NSIZE 16

void *pardo_init(length,S,P,D);
void *pardo_compute(length,S,newS,D,newD);
void *pardo_update(length,S,newS,D,newD);
void *pointerJumping (int* array, int length);
void handlerc(int rc,char* string, long t);
void join_threads(pthread_attr_t attr,pthread_t* threads,int t);

typedef struct thread_data1{
	int length;
	int value;
	int* S;
	int* P;
	int* D;
} thread_data1;

typedef struct thread_data2 {
	int length;
	int* S;
	int* newS;
	int* D;
	int* newD;
} thread_data2;

struct thread_data1 thread_data1_array[NUM_THREADS];
struct thread_data2 thread_data2_array[NUM_THREADS];

int main (int argc, char *argv[]) {
	//int array[] = {0,14,13,5,16,11,10,9,12,0,8,7,15,4,3,2,1};
	int array[] = {0,0,1,2,1,3,3,4,5,5,6,7,8,9,9,10,11};
	int length = NELEMS(array)-1;

	printf("Pointer Jumping result:\n\n");

	pthread_t threads[NUM_THREADS];
	pthread_attr_t attr;
	int rc;
	int t = 0;

	/* Initialize and set thread joinable attribute */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);


	pointerJumping(array,length);
	return 0;
}


void pointerJumping (int* P, int length) {
	// Using S and P to follow nomenclature of lecture slides
	int S[NSIZE] = {0};
	int D[NSIZE] = {0};
	int newS[NSIZE] = {0};
	int newD[NSIZE] = {0};
	int i,j;
	
	pardo_init(length,S,P,D);



	for (j=0; j<=log2(length); j++) {	
		printf("Iteration %d\n",j+1);
		
		pardo_compute(length,S,newS,D,newD);
		for (i=1; i<=length; i++) {
			printf("S[%d]=%d,%s",i,S[i],(S[i]>9)?" ":"  ");


			if (S[i] != S[S[i]]){
				newD[i] = D[i] + D[S[i]];
				newS[i] = S[S[i]];
			} else {
				newD[i] = D[i];
				newS[i] = S[i];
			}
		}
		printf("\n");
		pardo_update(length,S,newS,D,newD);
		for (i=1; i<=length; i++) {
			printf("D[%d]=%d,%s",i,D[i],(D[i]>9)?" ":"  ");
			S[i] = newS[i];
			D[i] = newD[i];
		}
		printf("\n\n");
	}
}


void *pardo_init(length,S,P,D){
	for (i=0; i<=length; i++) {
		S[i] = P[i];

		if (S[i] != 0)
			D[i] = 1;
		else
			D[i] = 0;
	}

}

void *pardo_compute(length,S,newS,D,newD){

}

void *pardo_update(length,S,newS,D,newD){

}

void handlerc(int rc,char* string, long t){
	if (rc) {
	  printf("ERROR; return code from pthread_%s(%ld) is %d\n",string,t,rc);
	  exit(-1);
	}
}

void join_threads(pthread_attr_t attr,pthread_t* threads,int t){
	int i = 0;
	int rc = 1;

	printf("Joining %d threads \n",t);
	for(i=0; i<t; i++) {
		pthread_attr_destroy(&attr);
		rc = pthread_join(threads[i],NULL);
		handlerc(rc,"join",i);
	}
}