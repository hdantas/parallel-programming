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
#define NMAX 32
#define NSIZE 1
int Ns[NSIZE] = {32};

typedef struct __ThreadArg {
	int min_i;
	int max_i;
} tThreadArg;


pthread_t callThd[NUM_THREADS];
pthread_mutex_t mutexpm;
pthread_barrier_t barr, internal_barr;

// Seed Input
int A[NMAX] = {58,89,32,73,131,156,30,29,141,37,133,151,88,53,122,126,131,49,130,115,16,83,40,145,10,112,20,147,14,104,111,92};
//int A[NMAX] = {10,5,6,7,3,4,2,1};

// Subset
int PM[NMAX]; //result preffix minima
int SM[NMAX]; //result suffix minima

//Auxiliar Functions
void prefixMinima (int min_i, int max_i);
void suffixMinima (int min_i, int max_i);
void printResult(char* string, int n);


void init(int n){
	/* Initialize the input for this iteration*/
	// B <- A

	int i;
	
	for (i = 0; i < n; i++) {
		PM[i] = A[i];
		SM[i] = A[i];
	}
}

void seq_function(int length){
	/* The code for sequential algorithm */
	// Perform operations on A and B
	int i;
	int currentmin = PM[0];
	
	//Preffix Minima
	for (i = 0; i < length; i++) {
		if (PM[i] < currentmin)
			currentmin = PM[i];	
		PM[i] = currentmin;
	}

	currentmin = SM[length-1];
	//Preffix Minima
	for (i = length-1; i > -1; i--) {
		
		if (SM[i] < currentmin)
			currentmin = SM[i];	
		SM[i] = currentmin;
	}	

}

void* par_function(void* tA){
	/* The code for threaded computation */
	
	tThreadArg *t = (tThreadArg *) tA;  //typecast threadarg

	int min_i = t -> min_i;
	int max_i = t -> max_i;
	// printf("min_i = %d, max_i = %d\n",min_i,max_i);
	prefixMinima(min_i,max_i);
	suffixMinima(min_i,max_i);

	pthread_exit(NULL);
}

int main (int argc, char *argv[])
{
	struct timeval startt, endt, result;
	int j, nt, t, n, c;
	
	int task, tasksCurrentLevel, totalTasks, id;
	int spawnedThreadsThisLevel ;
	int min_i, max_i, r;
	
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
		printf("| %3d | %8d |",n,TIMES);

		/* Run sequential algorithm */
		result.tv_usec=0;
		gettimeofday (&startt, NULL);
		for (t=0; t<TIMES; t++) {
			init(n);
			seq_function(n);
		}
		gettimeofday (&endt, NULL);
		// printResult("Sequential", n);

		result.tv_usec = (endt.tv_sec*1000000+endt.tv_usec) - (startt.tv_sec*1000000+startt.tv_usec);
		printf(" %ld.%06ld | ", result.tv_usec/1000000, result.tv_usec%1000000);

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

			for (t=0; t<TIMES; t++) //threaded execution
			{			
				init(n);
				totalTasks = n - 1;
				tasksCurrentLevel = n/2;
				task = 0;
				r = 2;
				

				while (task < totalTasks) {
					id = 1; spawnedThreadsThisLevel = 0; min_i = 0;

					while(spawnedThreadsThisLevel < tasksCurrentLevel) { // spawn threads
						task++;
						spawnedThreadsThisLevel++;
						max_i = min_i + r - 1;
						
						x[id-1].min_i = min_i;
						x[id-1].max_i = max_i;
						pthread_create(&callThd[id-1], &attr, par_function, (void *)&x[id-1]);
						// printf("Spawn thread %d/%d (index: %d -> %d). %d/%d/%d tasks assigned.\n",id,spawnedThreadsThisLevel,min_i,max_i,task,tasksCurrentLevel,totalTasks);
						id++;
										
						if(id > nt) {
							for (id = 1; id <= nt; id++) { // join all threads
								// printf("Ran out of threads! Join thread %d\n",id);
								pthread_join(callThd[id-1], &status);
							}
							id = 1;
						}
						min_i = max_i + 1;
					}

					for (j = 1; j < id; j++) { // join threads
						// printf("Finished level. Join thread %d\n",j);
						pthread_join(callThd[j-1], &status);
					}


					r *= 2;
					tasksCurrentLevel /= 2;
					// printf("\n");
				}
			}
			// printResult("Threaded", n);
			gettimeofday (&endt, NULL);

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
	printf("%s execution\n i: ",string);
	
	for (i = 0; i < n; i++)
		printf("%3d,",i);

	printf("\nPM: ");
	for (i = 0; i < n; i++)
		printf("%3d,",PM[i]);

	printf("\nSM: ");
	for (i = 0; i < n; i++)
		printf("%3d,",SM[i]);

	printf("\n");

}


void prefixMinima (int min_i, int max_i) {
	int min, i;
	int iLastleft = (max_i-min_i + 1)/2 + min_i - 1;
	int lastleft = PM[iLastleft];
	int iFirstright = iLastleft + 1;
	int firstright = PM[iFirstright];

	// printf("\tiLastleft = %d, lastleft = %d, iFirstright = %d, firstright = %d\n",iLastleft,lastleft,iFirstright,firstright);
	if (lastleft >= firstright){
		// printf("\tNo Change: %d >= %d\n",lastleft,firstright);
		return; //nothing to update, the array is correct
	} else {
		min = lastleft;
		for (i = iFirstright; i <= max_i; i++) {
			// printf("\tPM[%d] = %d\n",i,min);
			PM[i] = min;

			if (PM[i+1] <= min){
				return; //no need to update any longer
			}
		}
	}

}

void suffixMinima (int min_i, int max_i) {
	int min, i;
	int iLastleft = (max_i-min_i + 1)/2 + min_i - 1;
	int lastleft = SM[iLastleft];
	int iFirstright = iLastleft + 1;
	int firstright = SM[iFirstright];


	if (lastleft <= firstright) {
		// printf("\tNo Change: %d <= %d\n",lastleft,firstright);
		return; //nothing to update, the array is correct
	} else {
		min = firstright;
		for (i = iLastleft; i >= min_i; i--) {
			// printf("\tSM[%d] = %d\n",i,min);
			SM[i] = min;

			if(SM[i-1] <= min){
				break; //no need to update any longer
			}
		}
	}

}
