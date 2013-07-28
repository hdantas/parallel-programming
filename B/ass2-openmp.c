#include <stdio.h>
#include <string.h>
#include <omp.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

# define INT_MAX 32767

// Number of threads
#define NUM_THREADS 32

// Number of iterations
#define TIMES 1000

// Input Size
// #define NSIZE 7
// #define NMAX 262144
// int Ns[NSIZE] = {4096, 8192, 16384, 32768, 65536, 131072, 262144};
#define NSIZE 1
#define NMAX 32
int Ns[NSIZE] = {32};


// Seed Input
// int A[NMAX];
int I1[] = {28, 36, 40, 61, 68, 71,123, 149};
int I2[] = {2, 5, 18, 21, 24, 29, 31, 33, 34, 35, 47, 48, 49, 50, 52, 62, 66, 70, 73, 80, 88, 89, 114, 124, 125, 131, 143, 144, 145, 148, 155, 159};

// Subset
int A[NMAX]; // input 1
int B[NMAX]; // input 2
int C[2*NMAX]; //result
int oldC[2*NMAX];

//Auxiliar Functions
int rank (int value, int* array, int sizeArray);
void printResult(char* string);
void seqmerge (int i, int j);
void sort(int* array, int length);

//Auxiliar variables
int a, b, sizeA, sizeB;
int AA[NMAX];
int BB[NMAX];

void init(int n){
	/* Initialize the input for this iteration*/
	// A <- I1
	// B <- I2
	int i;
	sizeA = (sizeof(I1) / sizeof(I1[0]));
	sizeB = (sizeof(I2) / sizeof(I2[0]));

	for (i = 0; i < NMAX; i++) {
		if (i < sizeA)
			A[i] = I1[i];
		else
			A[i] = -1;
		
		if (i < sizeB)
			B[i] = I2[i];
		else
			B[i] = -1;

		AA[i] = -1;
		BB[i] = -1;
		C[2*i] = -1; // set C[2*NMAX] = {-1}
		C[2*i+1] = -1;
		oldC[2*i] = -1;
		oldC[2*i+1] = -1;		
	}

	
	a = floor(log2(sizeA));
	b = floor(log2(sizeB));

	AA[0] = 0;
	BB[sizeB/b] = sizeA;
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
}

void omp_function(int length, int nthreads){
	/* The code for threaded computation */
	// Perform operations on B
	int i,j, chunk;


	
	// Determine min_i and max_i for this thread
	int n_A, n_B;

	//for input A
	n_A = floor(sizeA/a);

	//for input B
	n_B = floor(sizeB/b);

	if (n_A > n_B)
		chunk = ceil(n_A/nthreads);
	else
		chunk = ceil(n_B/nthreads);

	if (chunk == 0)
		chunk = 1;

	// printf("chunk = %d, n_A = %d, n_B = %d, nthreads = %d\n",chunk,n_A,n_B,nthreads);
	#pragma omp parallel num_threads(nthreads) shared(A,AA,B,BB,oldC) private(i,j)
	{
		// Step 2
		j = 1;
		#pragma omp for schedule(dynamic,chunk) nowait
		for (i = 1; i <= length; i++){
			if (i <= n_A)
				AA[i] = rank(A[i*a-1] - 1, B, sizeB);
			if (j <= n_B)
				BB[j-1] = rank(B[j*b-1], A, sizeA);
			j++;
		}

		// Step 3
		j = 1;
		#pragma omp for schedule(dynamic,chunk)
		for (i = 1; i <= length; i++){
			if (i <= n_A) {
				C[AA[i] + a*i - 1] = A[a*i-1];
				oldC[AA[i] + a*i - 1] = A[a*i-1];
			}
			if (j <= n_B) {
				C[BB[j-1] + b*j - 1] = B[b*j-1];
				oldC[BB[j-1] + b*j - 1] = B[b*j-1];
			}
			j++;
		}


		//Step 4
		j = 1;
		#pragma omp for schedule(dynamic,chunk) nowait
		for (i = 0; i <= length; i++){
			if (i <= n_A)
				seqmerge(i*a , AA[i]);
			if (j <= n_B)
				seqmerge(BB[j-1], j*b);
			j++;
		}
	}
}

int main (int argc, char *argv[])
{
	struct timeval startt, endt, result;
	int nt, t, n, c;
	
 	result.tv_sec = 0;
 	result.tv_usec= 0;

	/* Generate a seed input */
	// srand ( time(NULL) );
	// for(k=0; k<NMAX; k++){
	// 	A[k] = rand();
	// }

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
		// printResult("Sequential");

		result.tv_usec = (endt.tv_sec*1000000+endt.tv_usec) - (startt.tv_sec*1000000+startt.tv_usec);
		printf(" %ld.%06ld | ", result.tv_usec/1000000, result.tv_usec%1000000);

		/* Run threaded algorithm(s) */
		for(nt=1; nt<NUM_THREADS; nt=nt<<1){

			result.tv_sec=0; result.tv_usec=0;
			gettimeofday (&startt, NULL);
			for (t=0; t<TIMES; t++)
			{			
				init(n);
				omp_function(n,nt);
			}
			gettimeofday (&endt, NULL);

			// printResult("Threaded");
 			result.tv_usec += (endt.tv_sec*1000000+endt.tv_usec) - (startt.tv_sec*1000000+startt.tv_usec);
			printf(" %ld.%06ld | ", result.tv_usec/1000000, result.tv_usec%1000000);
		}
		printf("\n");
	}
	return 0;
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
	printf("\n%s algorithm\n",string	);
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

	// if(string[0] == 'S' || string[0] == 's') //sequential algorithm
	// 	return;
	
	// //threaded
	// for (i = 0; i < sizeA; i++){
	// 	printf("AA[%d]=%d,%s",i,AA[i],(AA[i]>9)?" ":"  ");
	// }
	// printf("\n");
	// for (i = 0; i < sizeB; i++){
	// 	printf("BB[%d]=%d,%s",i,BB[i],(BB[i]>9)?" ":"  ");
	// }
	// printf("\n\n");

	// printf("a = %d\t sizeA = %d\n",a,sizeA);
	// printf("b = %d\t sizeB = %d\n",b,sizeB);
	// printf("\n\n");

}

void seqmerge (int i, int j){

	int k, x;
	int beginC  = i + j;
	int endC = beginC;
	int tempC[NMAX] = {-1};
	
	// printf("\n(%d, %d)\n",i,j);
	// for (k = 0; k < sizeA + sizeB; k++){
	// 	if (oldC[k]!= -1)
	// 		printf("oldC[%d] = %d, ",k,oldC[k]);
	// }
	
	// printf("\n");
	x = 0;
	while(1){
		if (i < sizeA){
			tempC[x++] = A[i++]; //merge first, sort later
		}
		else{
			tempC[x++] = -1;
		}

		if (j < sizeB)
			tempC[x++] = B[j++]; //merge first, sort later
		else{
			tempC[x++] = -1;
		}

		endC++;

		if(i >= sizeA && j >= sizeB){
			break;
		}

		if (oldC[endC] != -1){
			endC--;
			break;
		}
	}

	// printf("\nbeginC = %d, endC = %d, i + j - 2 = %d, sizeA+sizeB = %d, x = %d\n",beginC, endC, i + j - 2, sizeA+sizeB, x);
	// for (k = 0; k < x; k++)
	// 	printf("tempC[%d] = %d, ",k,tempC[k]);

	// printf("\n");
	// sort(tempC,2*(endC - beginC + 1));
	sort(tempC,x);
	for (k = beginC; k <= endC; k++)
	{
		C[k] = tempC[k-beginC];
		// printf("C[%d] = %d, ",k,C[k]);
	}
	// printf("\n\n");

}

void sort(int* array, int length){
	int i, changed, temp;

	do{
		changed = 0;  
		for (i = 0; i < length - 1; i++){
			if (((array[i] > array[i+1]) && array[i+1]!=-1) || (array[i] == -1 && array[i+1] != -1)){ //values -1 should be pushed to the end of the array
				changed = 1;
				temp = array[i];
				array[i] = array[i+1];
				array[i+1] = temp;
			}
		}
	} while (changed == 1);

	// printf("\n");
	// for (i = 0; i < length; i++)
	// {
	// 	printf("array[%d] = %d ",i,array[i]);
	// }
	// printf("\n");

}