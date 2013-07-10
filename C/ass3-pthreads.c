#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_THREADS 10
#define NELEMS(x)  (sizeof(x) / sizeof(x[0])) // number of elements in an array
#define NSIZE 16

void *pardo_init(void *threadarg);
void *pardo_compute(void *threadarg);
void *pardo_update(void *threadarg);
void pointerJumping (int* array, int length);
void handlerc(int rc,char* string, long t);
void join_threads(pthread_attr_t attr,pthread_t* threads,int t);

typedef struct thread_data1{
	int id;
	int it;
	int* S;
	int* P;
	int* D;
} thread_data1;

typedef struct thread_data2 {
	int id;
	int it;	
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




	pointerJumping(array,length);
	return 0;
}


void pointerJumping (int* P, int length) {
	// Using S and P to follow nomenclature of lecture slides
	int S[NSIZE+1] = {0};
	int D[NSIZE+1] = {0};
	int newS[NSIZE+1] = {0};
	int newD[NSIZE+1] = {0};
	int i,j,t,rc;
	t = 0;
	
	pthread_t threads[NUM_THREADS];
	pthread_attr_t attr;

	/* Initialize and set thread joinable attribute */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	// pardo_init
	// printf("pardo_init\n");
	for (i=0; i<=length; i++) {
		thread_data1_array[t] = (thread_data1) {
				.id = t,
				.it = i,
				.S = S,
				.P = P,
				.D = D
		};

		rc = pthread_create(&threads[t], &attr, pardo_init, (void *) &thread_data1_array[t]);
		handlerc(rc,"init",0);
			
		t++;
		if(t == NUM_THREADS){
			join_threads(attr,threads,t);
			t = 0;
		}
	}
	join_threads(attr,threads,t);

	t = 0;
	j = 0;
	for (j=0; j<=floor(log2(length)); j++) {
		printf("Iteration %d\n",j+1);
		for (i=1; i<=length; i++){
			printf("S[%d]=%d,%s",i,S[i],(S[i]>9)?" ":"  ");
		}
		printf("\n");

		// printf("pardo_compute\n");
		for (i=1; i<=length; i++) {
			// pardo compute
			thread_data2_array[t] = (thread_data2){
				.id = t,
				.it = i,
				.S = S,
				.newS = newS,
				.D = D,
				.newD = newD
			};

			rc = pthread_create(&threads[t], &attr, pardo_compute, (void *) &thread_data2_array[t]);
			handlerc(rc,"compute",0);
			t++;
			if(t == NUM_THREADS){
				join_threads(attr,threads,t);
				t = 0;
			}
		}

		join_threads(attr,threads,t);
		t = 0;

		for (i=1; i<=length; i++) {
			printf("D[%d]=%d,%s",i,D[i],(D[i]>9)?" ":"  ");
		}
		printf("\n\n");

		for (i=1; i<=length; i++) {
			// pardo update
			thread_data2_array[t] = (thread_data2){
				.id = t,
				.it = i,
				.S = S,
				.newS = newS,
				.D = D,
				.newD = newD
			};

			rc = pthread_create(&threads[t], &attr, pardo_update, (void *) &thread_data2_array[t]);
			handlerc(rc,"update",0);
			t++;
			if(t == NUM_THREADS){
				join_threads(attr,threads,t);
				t = 0;
			}
		}

		join_threads(attr,threads,t);
		t = 0;


	}

}


void *pardo_init(void *threadarg){
	thread_data1 *t1 = (thread_data1*) threadarg;  //typecast threadarg

	int i = t1->it;
	int* S = t1->S;
	int* P = t1->P;
	int* D = t1->D;
	
	S[i] = P[i];
	if (S[i] != 0)
		D[i] = 1;
	else
		D[i] = 0;

	pthread_exit(NULL);
}

void *pardo_compute(void *threadarg){
	thread_data2 *t2 = (thread_data2*) threadarg;  //typecast threadarg
	int i = t2->it;
	int* S = t2->S;
	int* newS = t2->newS;
	int* D = t2->D;
	int* newD = t2->newD;

	if (S[i] != S[S[i]]){
		newD[i] = D[i] + D[S[i]];
		newS[i] = S[S[i]];
	} else {
		newD[i] = D[i];
		newS[i] = S[i];
	}
	pthread_exit(NULL);
}

void *pardo_update(void *threadarg){
	thread_data2 *t2 = (thread_data2*) threadarg;  //typecast threadarg
	
	int i = t2->it;
	int* S = t2->S;
	int* newS = t2->newS;
	int* D = t2->D;
	int* newD = t2->newD;
	
	S[i] = newS[i];
	D[i] = newD[i];
	pthread_exit(NULL);

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

	// printf("Joining %d threads \n",t);
	for(i=0; i<t; i++) {
		pthread_attr_destroy(&attr);
		rc = pthread_join(threads[i],NULL);
		handlerc(rc,"join",i);
	}
}