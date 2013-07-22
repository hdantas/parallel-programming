#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

# define INT_MAX 32767

// Number of threads
#define NUM_THREADS 32

// Number of iterations
#define TIMES 1//000

// Input Size
// #define NSIZE 7
// #define NMAX 262144
// int Ns[NSIZE] = {4096, 8192, 16384, 32768, 65536, 131072, 262144};
#define NSIZE 1
#define NMAX 32
int Ns[NSIZE] = {32};

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
int I1[] = {28, 36, 40, 61, 68, 71,123, 149};
int I2[] = {2, 5, 18, 21, 24, 29, 31, 33, 34, 35, 47, 48, 49, 50, 52, 62, 66, 70, 73, 80, 88, 89, 114, 124, 125, 131, 143, 144, 145, 148, 155, 159};

// Subset
int A[NMAX]; // input 1
int B[NMAX]; // input 2
int C[2*NMAX]; //result

//Auxiliar Functions
int rank (int value, int* array, int sizeArray);
void printResult(char* string);
void seqmerge (int minA, int maxA, int minB, int maxB, int minC);

//Auxiliar variables
int a, b, sizeA, sizeB;
int AA[NMAX] = {0};
int BB[NMAX] = {0};

void init(int n){
	/* Initialize the input for this iteration*/
	// A <- I1
	// B <- I2
	int i;
	sizeA = (sizeof(I1) / sizeof(I1[0]));
	sizeB = (sizeof(I2) / sizeof(I2[0]));

	for (i = 0; i < sizeA; i++) {
		A[i] = I1[i];
	}

	for (i = 0; i < sizeB; i++) {
		B[i] = I2[i];
	}

	for (i = 0; i < (sizeA+sizeB); i++) { //reset result
		C[i] = 0;
	}

	a = floor(log2(sizeA));
	b = floor(log2(sizeB));

}

void seq_function(int length){
	/* The code for sequential algorithm */
	// Perform operations on A and B
	int i, j, k = 0;
	int tmpmin = INT_MAX;

	for (i=0, j = 0; i < sizeA || j < sizeB;){
		if (i < sizeA && j < sizeB) {
			if (A[i] <= B[j]) {
				tmpmin = A[i++];
			} else {
				tmpmin = B[j++];
			}
		} else if (i < sizeA) {
			tmpmin = A[i++];
		} else if (j < sizeB) {
			tmpmin = B[j++];
		} else {
			printf("Hm ... This was not supposed to happen!\n");
		}
		C[k++] = tmpmin;
	}

	printResult("Sequential");
}

void* par_function(void* tA){
	/* The code for threaded computation */
	// Perform operations on B
	
	tThreadArg *t = (tThreadArg *) tA;  //typecast threadarg
	int id = t -> id; // thread id
	int nrT = t -> nrT; // number of threads in this round
	int n = t -> n; //input size
	
	int minA = 0;
	int minB = 0;
	int minC = 0;
	int maxA = 0;
	int maxB = 0;
	int indexA = 0;
	int indexB = 0;
	int AAi[sizeA];
	int BBi[sizeB];
	int i, j ;


	// Determine min_i and max_i for this thread
	int minA_i, maxA_i, minB_j, maxB_j, delta, r, n_A, n_B;

	//for input A
	n_A = floor(sizeA/a);
	r = floor(n_A/nrT);
	delta = n_A - nrT*r;
	if (id <= delta){
		minA_i = r*(id-1)+id;
		maxA_i = r + minA_i;
	} else {
		minA_i = r*(id-1) + delta + 1;
		maxA_i = minA_i + r - 1;
	}

	//for input B
	n_B = floor(sizeB/b);
	r = floor(n_B/nrT);
	delta = n_B - nrT*r;
	if (id <= delta){
		minB_j = r*(id-1)+id;
		maxB_j = r + minB_j;
	} else {
		minB_j = r*(id-1) + delta + 1;
		maxB_j = minB_j + r - 1;
	}

	printf("thread %d\tminA_i = %d, maxA_i = %d\tminB_j = %d, maxB_j = %d\n",id,minA_i,maxA_i,minB_j,maxB_j);
	//Step 2
	for (i = minA_i; i <= maxA_i; i++){
		AA[i-1] = rank(A[i*a-1] - 1, B, sizeB);
		AAi[i-1] = i*a-1;
	}
	for (j = minB_j; j <= maxB_j; j++){
		BB[j-1] = rank(B[j*b-1], A, sizeA);
		BBi[j-1] = j*b;
	}

	//Step 3
	for (i = minA_i; i <= maxA_i; i++){
		C[AA[i] + a*i ] = A[a*i];
	}

	for (j = minB_j; j <= maxB_j; j++){
		C[BB[i] + b*i] = B[b*i];
	}

	pthread_barrier_wait(&internal_barr);

/*********************************************  UNDERSTAND WHAT IT DOES AND ADJUST PART BELOW FOR PARALLEL EXECUTION  ********************************************************/
	// for (i = 0; i <= a+b; i++) {
	// 	minC = maxA + maxB + 2;
		
	// 	if (maxA != -1)
	// 		minA = maxA+1;
	// 	if (maxB != -1)
	// 		minB = maxB+1;

	// 	// printf("indexA = %d\tindexB = %d\n",indexA,indexB);
		
	// 	if (i == 0) {
	// 		minA = 0;
	// 		minB = 0;
	// 		minC = 0;
	// 	}
		
		
	// 	if (((BB[indexB] <= AAi[indexA]) && (indexB <= sizeB/b-1)) || (indexA > sizeA/a-1)) {
	// 		// printf("maxA = BB[%d]\t",indexB);
	// 		maxA = BB[indexB]-1;
	// 	} else {
	// 		// printf("maxA = AAi[%d]\t",indexA);
	// 		maxA = AAi[indexA]-1;
	// 	}

	// 	if (((BBi[indexB] <= AA[indexA]) && (indexB <= sizeB/b-1)) || (indexA > sizeA/a-1)) {
	// 		// printf("maxB = BBi[%d]\t",indexB);
	// 		maxB = BBi[indexB++]-1;
	// 	} else {
	// 		// printf("maxB = AA[%d]\t",indexA);
	// 		maxB = AA[indexA++]-1;
	// 	}

	// 	if(i == a+b) { //last run
	// 		maxA = sizeA-1;
	// 		maxB = sizeB-1;
	// 	}

	// 	// printf("minA = %d\tmaxA = %d\tminB = %d\tmaxB = %d\tminC = %d\tA\tB\tC\n\n",minA,maxA,minB,maxB,minC);
	// 	seqmerge (minA,maxA,minB,maxB,minC);
	// }

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

		/* Run threaded algorithm(s) */
		for(nt=1; nt<NUM_THREADS; nt=nt<<1){
			printf("%d threads\n",nt);
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
				printResult("Threaded");

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


int rank (int value, int* array, int sizeArray){
	int i = 0, cnt = 0;
	for (i=0; i<sizeArray; i++){
		if (*(array+i) <= value)
			cnt++;
	}
	return cnt;
}


void printResult(char* string){
	int i;
	printf("%s algorithm\n",string	);
	for (i = 0; i < sizeA; i++){
		printf("A[%d]=%d,%s",i,A[i],(A[i]>9)?" ":"  ");
	}
	printf("\n");
	for (i = 0; i < sizeB; i++){
		printf("B[%d]=%d,%s",i,B[i],(B[i]>9)?" ":"  ");
	}
	printf("\n");
	for (i = 0; i < (sizeA+sizeB); i++){
		printf("C[%d]=%d,%s",i,C[i],(C[i]>9)?" ":"  ");
	}
	printf("\n\n");

	if(string[0] == 'S') //sequential algorithm
		return;
	
	//threaded
	for (i = 0; i < sizeA; i++){
		printf("AA[%d]=%d,%s",i,AA[i],(AA[i]>9)?" ":"  ");
	}
	printf("\n");
	for (i = 0; i < sizeB; i++){
		printf("BB[%d]=%d,%s",i,BB[i],(BB[i]>9)?" ":"  ");
	}
	printf("\n\n");

	printf("a = %d\t sizeA = %d\n",a,sizeA);
	printf("b = %d\t sizeB = %d\n",b,sizeB);
	printf("\n\n");

}

void seqmerge (int minA, int maxA, int minB, int maxB, int minC){

	int indexA = minA;
	int indexB = minB;
	int maxC = minC + (maxA-minA) + (maxB-minB) + 1;
	int i = 0;

	// printf("minA = %d\tminB = %d\tminC = %d\t",minA,minB,minC);
	if (maxA == -1) {
		// printf("maxC = %d\n",maxC);
		for (i = minC; i <= maxC; i++) {
			C[i] = B[indexB++];
			// printf("C[%d] = %d\n",i,*(arrayC+i));
		}

	} else if (maxB == -1){
		// printf("maxC = %d\n",maxC);
		for (i = minC; i <= maxC; i++){
			C[i] = A[indexA++];
			// printf("C[%d] = %d\n",i,*(arrayC+i));
		}
	
	} else {
		// printf("maxC = %d\n",maxC);
		for (i = minC; i <= maxC; i++) {
			if (((A[indexA] < B[indexB]) && (indexA <= maxA)) || (indexB > maxB)) {
				C[i] = A[indexA++];
			} else if (((B[indexB] <= A[indexA]) && (indexB <= maxB)) || (indexA > maxA)) {
				C[i] = B[indexB++];
			}
			// printf("C[%d] = %d\n",i,*(arrayC+i));
		}
	}
}