#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

// Number of threads
#define NUM_THREADS 32

// Number of iterations
#define TIMES 1000

// Input Size
// #define NSIZE 7
// #define NMAX 262144
// int Ns[NSIZE] = {4096, 8192, 16384, 32768, 65536, 131072, 262144};
#define NSIZE 1
#define NMAX 16
int Ns[NSIZE] = {16};

typedef struct __ThreadArg {
	int id;
	int nrT;
	int n;
} tThreadArg;


pthread_t callThd[NUM_THREADS];
pthread_mutex_t mutexpm;
pthread_barrier_t barr, internal_barr;

// Seed Input
// int A[NMAX];
int A[] = {0,14,13,5,16,11,10,9,12,0,8,7,15,4,3,2,1};

// Subset
int B[NMAX];

//Auxiliar Arrays
int S[NMAX+1] = {0};
int D[NMAX+1] = {0};
int newS[NMAX+1] = {0};
int newD[NMAX+1] = {0};

void printResult(char* string, int n);

void init(int n){
	/* Initialize the input for this iteration*/
	// B <- A
	int i = 0;
	for (i = 0; i <= n; i++) {
		B[i] = A[i];
	}
}

void seq_function(int length){
	/* The code for sequential algorithm */
	// Perform operations on B

	int i,j;
	
	for (i=0; i<=length; i++) {
		S[i] = B[i];

		if (0 != S[i])
			D[i] = 1;
		else
			D[i] = 0;
	}

	for (j=0; j<=log2(length); j++) {	
		// printf("Iteration %d\n",j+1);
		for (i=1; i<=length; i++) {
			// printf("S[%d]=%d,%s",i,S[i],(S[i]>9)?" ":"  ");

			if (S[i] != S[S[i]]) {
				newD[i] = D[i] + D[S[i]];
				newS[i] = S[S[i]];
			} else {
				newD[i] = D[i];
				newS[i] = S[i];
			}
		}
		// printf("\n");
		for (i=1; i<=length; i++) {
			// printf("D[%d]=%d,%s",i,D[i],(D[i]>9)?" ":"  ");
			S[i] = newS[i];
			D[i] = newD[i];
		}
		// printf("\n\n");	
	}
}

void* par_function(void* a){
	/* The code for threaded computation */
	// Perform operations on B
	
	tThreadArg *t = (tThreadArg *) a;  //typecast threadarg
	int id = t->id; // thread id
	int nrT = t->nrT; // number of threads in this round
	int n = t->n; //input size


	int i, j ;
	int max_i = id*(n/nrT);
	int min_i = (id-1)*(n/nrT)+1;

	for (i=min_i; i<=max_i; i++) {
		S[i] = B[i];

		if (0 != S[i])
			D[i] = 1;
		else
			D[i] = 0;
	}

	// printf("\tt%d\t1: b4 internal_barr\n",id);
	pthread_barrier_wait(&internal_barr);
	// printf("\tt%d\t1: after internal_barr\n",id);
	
	for (j=0; j<=log2(n); j++) {
		// printf("Iteration %d\n",j+1);
		for (i=min_i; i<=max_i; i++) {
			// printf("S[%d]=%d,%s",i,S[i],(S[i]>9)?" ":"  ");

			if (S[i] != S[S[i]]){
				newD[i] = D[i] + D[S[i]];
				newS[i] = S[S[i]];
			} else {
				newD[i] = D[i];
				newS[i] = S[i];
			}
		}
		// printf("\n");
		pthread_barrier_wait(&internal_barr);
		for (i=min_i; i<=max_i; i++) {
			// printf("D[%d]=%d,%s",i,D[i],(D[i]>9)?" ":"  ");
			S[i] = newS[i];
			D[i] = newD[i];
		}
		// printf("\tt%d\t2: b4 internal_barr\n",id);
		pthread_barrier_wait(&internal_barr);
		// printf("\tt%d\t2: after internal_barr\n",id);
	}
	
	pthread_barrier_wait(&barr);

	pthread_exit(NULL);

}

int main (int argc, char *argv[])
{
	struct timeval startt, endt, result;
	int j, nt, t, n, c;
	void *status;
 	pthread_attr_t attr;
	tThreadArg x[NUM_THREADS];
	
 	result.tv_sec = 0;
 	result.tv_usec= 0;

	/* Generate a seed input */
	// srand ( time(NULL) );
	// for(k=0; k<NMAX; k++){
	// 	A[k] = rand();
	// }

 	/* Initialize and set thread detached attribute */
 	pthread_attr_init(&attr);
 	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	printf("|NSize|Iterations|    Seq   |    Th01   |    Th02   |    Th04   |    Th08   |   Par16   |\n");

	// for each input size
	for(c=0; c<NSIZE; c++){
		n=Ns[c];
		printf("| %03d | %08d |",n,TIMES);

		/* Run sequential algorithm */
		result.tv_usec=0;
		gettimeofday (&startt, NULL);
		for (t=0; t<TIMES; t++) {
			init(n);
			seq_function(n);
		}
		gettimeofday (&endt, NULL);
		result.tv_usec = (endt.tv_sec*1000000+endt.tv_usec) - (startt.tv_sec*1000000+startt.tv_usec);
		printf(" %ld.%06ld | ", result.tv_usec/1000000, result.tv_usec%1000000);
		// printResult("sequential",n);

		/* Run threaded algorithm(s) */
		for(nt=1; nt<NUM_THREADS; nt=nt<<1){
			if(pthread_barrier_init(&barr, NULL, nt+1))
			{
				printf("Could not create a barrier\n");
				return -1;
			}

			if(pthread_barrier_init(&internal_barr, NULL, nt))
			{
				printf("Could not create a barrier\n");
				return -1;
			}

			result.tv_sec=0; result.tv_usec=0;
			gettimeofday (&startt, NULL);
			for (t=0; t<TIMES; t++)
			{			
				init(n);
				for (j=1; j<=/*NUMTHRDS*/nt; j++)
				{
					x[j-1].id = j; 
					x[j-1].nrT=nt; // number of threads in this round
					x[j-1].n=n; //input size
					pthread_create(&callThd[j-1], &attr, par_function, (void *)&x[j-1]);
				}
				pthread_barrier_wait(&barr);
	
				/* Wait on the other threads */
				for(j=0; j</*NUMTHRDS*/nt; j++)
				{
					pthread_join(callThd[j], &status);
				}

			}
			gettimeofday (&endt, NULL);
			// printResult("threaded",n);

			if (pthread_barrier_destroy(&barr)) {
					printf("Could not destroy the barrier\n");
					return -1;
			}
			if (pthread_barrier_destroy(&internal_barr)) {
					printf("Could not destroy the barrier\n");
					return -1;
			}
 			result.tv_usec += (endt.tv_sec*1000000+endt.tv_usec) - (startt.tv_sec*1000000+startt.tv_usec);
			printf(" %ld.%06ld | ", result.tv_usec/1000000, result.tv_usec%1000000);
		}
		printf("\n");
	}
	pthread_exit(NULL);
}

void printResult(char* string, int n){
	int i;
	
	for (i = 1; i <= n; i++){
		printf("S[%d]=%d,%s",i,S[i],(S[i]>9)?" ":"  ");
	}
	printf("\n");
	for (i = 1; i <= n; i++){
		printf("D[%d]=%d,%s",i,D[i],(D[i]>9)?" ":"  ");
	}
	printf("\n\n");
}