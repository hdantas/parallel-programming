#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_THREADS 10
#define MAX_INPUT_SIZE 64
#define NELEMS(x)  (sizeof(x) / sizeof(x[0])) // number of elements in an array

void seqmerge (int minA, int maxA, int minB, int maxB, int minC, int* arrayA, int* arrayB, int* arrayC);
int rank (int value, int* array, int sizeArray);
void *pardo_rank(void *threadarg);
void *pardo_seqmerge(void *threadarg);
void handlerc(int rc,char* string, long t);
void join_threads(pthread_attr_t attr,pthread_t* threads,int t);

typedef struct thread_data1{
	int id;
	int value;
	int* array;
	int sizeArray;
	int* destination;
} thread_data1;

typedef struct thread_data2 {
	int id;
	int minA;
	int maxA;
	int minB;
	int maxB;
	int minC;
	int* A;
	int* B;
	int* C;
} thread_data2;

struct thread_data1 thread_data1_array[NUM_THREADS];
struct thread_data2 thread_data2_array[NUM_THREADS];

int main (int argc, char *argv[]) {
	int A[] = {28, 36, 40, 61, 68, 71,123, 149};
	int B[] = {2, 5, 18, 21, 24, 29, 31, 33, 34, 35, 47, 48, 49, 50, 52, 62, 66, 70, 73, 80, 88, 89, 114, 124, 125, 131, 143, 144, 145, 148, 155, 159};

	pthread_t threads[NUM_THREADS];
	pthread_attr_t attr;
	int rc;
	int t = 0;

	/* Initialize and set thread joinable attribute */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);


	int sizeA = NELEMS(A);
	int sizeB = NELEMS(B);
	int sizeAB = sizeA+sizeB;

	
	int AA[sizeA];
	int BB[sizeB];
	int AAi[sizeA];
	int BBi[sizeB];

	int C[sizeAB];
	int minA = 0;
	int minB = 0;
	int minC = 0;
	int maxA = 0;
	int maxB = 0;

	int a = floor(log2(sizeA));
	int b = floor(log2(sizeB));
	
	int i = 0;
	int j = 0;

	printf("STEP1:\na = %d\tb = %d\tsizeA = %d\tsizeB = %d\n",a,b,sizeA,sizeB);

	printf("\nSTEP2:\n");
	for (i = 1; i <= sizeA/a; i++){
		thread_data1_array[t] = (thread_data1) {.id = t,.value = A[i*a-1]-1,.array = B,.sizeArray = sizeB,.destination = &(AA[i-1])};
		rc = pthread_create(&threads[t], &attr, pardo_rank, (void *) &thread_data1_array[t]);
		handlerc(rc,"create",0);
		t++;
		if(t == NUM_THREADS){
			join_threads(attr,threads,t);
			t = 0;
		}
		// AA[i-1] = rank(A[i*a-1]-1,B,sizeB);
		// printf("rank(%d,B,%d)\t",A[i*a-1]-1,sizeB);
		AAi[i-1] = i*a-1;
	}


	for (j = 1; j <= sizeB/b; j++){
		thread_data1_array[t] = (thread_data1) {.id = t,.value = B[j*b-1],.array = A,.sizeArray = sizeA,.destination = &(BB[j-1])};
		rc = pthread_create(&threads[t], &attr, pardo_rank, (void *) &thread_data1_array[t]);
		handlerc(rc,"create",0);
		t++;
		if(t == NUM_THREADS){
			join_threads(attr,threads,t);
			t = 0;
		}
		// BB[j-1] = rank(B[j*b-1],A,sizeA);
		// printf("rank(%d,A,%d)\t",B[j*b-1],sizeA);
		BBi[j-1] = j*b;
	}

	join_threads(attr,threads,t);
	t = 0;

	for (i = 1,j=1; i <= sizeA/a; i++){
		printf("AAi[%d] = %d\tAA[%d] = %d\n",i,AAi[i-1],i,AA[i-1]);
	}
	for (j = 1; j <= sizeB/b; j++){	
		printf("BBi[%d] = %d\tBB[%d] = %d\n",j,BBi[j-1],j,BB[j-1]);

	}
		
	// printf("\nSTEP 3:\n");
	// for (i = 1; i <= sizeA/a; i++) {
	// 	C[AA[i-1]+a*i-1] = A[a*i-1];
	// 	printf("C[%d] = %d\n",AA[i-1]+a*i,C[AA[i-1]+a*i-1]);
	// 	// C_assigned[cnt++] = AA[i]+a*i;
	// }

	// for (j = 1; j <= sizeB/b; j++){
	// 	C[BB[j-1]+b*j-1] = B[b*j-1];
	// 	printf("C[%d] = %d\n",BB[j-1]+b*j,C[BB[j-1]+b*j-1]);
	// 	// C_assigned[cnt++] = BB[j]+b*j;
	// }
	
	minC = AA[i-1] + BB[i-1];
	// printf("seqmerge(%d,%d,%d,%d,%d,A,B,C)\n",BB[i-1],BB[i]-1,AA[i-1],AA[i]-1,minC);	
	// seqmerge(BB[i-1],BB[i]-1,AA[i-1],AA[i]-1,minC,A,B,C);
	
	printf("\nSTEP 4:\n");
	int indexA = 0;
	int indexB = 0;

	for (i = 0; i <= a+b; i++) {
		
		minC = maxA + maxB + 2;
		
		if (maxA != -1)
			minA = maxA+1;
		if (maxB != -1)
			minB = maxB+1;

		// printf("indexA = %d\tindexB = %d\n",indexA,indexB);
		
		if (i == 0) {
			minA = 0;
			minB = 0;
			minC = 0;
		}
		
		
		if (((BB[indexB] <= AAi[indexA]) && (indexB <= sizeB/b-1)) || (indexA > sizeA/a-1)) {
			// printf("maxA = BB[%d]\t",indexB);
			maxA = BB[indexB]-1;
		} else {
			// printf("maxA = AAi[%d]\t",indexA);
			maxA = AAi[indexA]-1;
		}

		if (((BBi[indexB] <= AA[indexA]) && (indexB <= sizeB/b-1)) || (indexA > sizeA/a-1)) {
			// printf("maxB = BBi[%d]\t",indexB);
			maxB = BBi[indexB++]-1;
		} else {
			// printf("maxB = AA[%d]\t",indexA);
			maxB = AA[indexA++]-1;
		}

		if(i == a+b) { //last run
			maxA = sizeA-1;
			maxB = sizeB-1;
		}


		// printf("minA = %d\tmaxA = %d\tminB = %d\tmaxB = %d\tminC = %d\tA\tB\tC\n\n",minA,maxA,minB,maxB,minC);
		
		// seqmerge (minA,maxA,minB,maxB,minC,A,B,C);
		thread_data2_array[t] = (thread_data2) {.id = t,.minA = minA, .maxA = maxA, .minB = minB, .maxB = maxB, .minC = minC, .A = A, .B = B, .C = C};
		rc = pthread_create(&threads[t], &attr, pardo_seqmerge, (void *) &thread_data2_array[t]);
		handlerc(rc,"create",0);
		t++;
		if(t == NUM_THREADS){
			join_threads(attr,threads,t);
			t = 0;
		}


	}

	
	join_threads(attr,threads,t);
	t = 0;
	printf("\n");
	for (i = 0; i < sizeAB; i++) {
		printf("C[%d] = %d\n",i,C[i]);
	}
		
	// seqmerge(2,2,1,3,3,A,B,C);
	// printf("C[3] = %d\tC[4] = %d\tC[5] = %d\tC[6] = %d\n",C[3],C[4],C[5],C[6]);
	
	// printf("Rank(A:AB)\n");
	// for (i=0; i < sizeA; i++){
	// 	*(resultA + i) = rank (*(arrayA+i), arrayAB, sizeAB);
	// 	printf("%d%s",*(resultA+i),(i==(sizeA-1))?"\n":", ");
	// }

	// printf("\nRank(B:AB)\n");
	// for (i=0; i < sizeB; i++){
	// 	*(resultB + i) = rank (*(arrayB+i), arrayAB, sizeAB);
	// 	printf("%d%s",*(resultB+i),(i==(sizeB-1))?"\n":", ");
	// }
	pthread_exit (NULL);
	return 0;
}

void seqmerge (int minA, int maxA, int minB, int maxB, int minC, int* arrayA, int* arrayB, int* arrayC){

	int indexA = minA;
	int indexB = minB;
	int maxC = minC + (maxA-minA) + (maxB-minB) + 1;
	int i = 0;

	// printf("minA = %d\tminB = %d\tminC = %d\t",minA,minB,minC);
	if (maxA == -1) {
		// printf("maxC = %d\n",maxC);
		for (i = minC; i <= maxC; i++) {
			*(arrayC+i) = *(arrayB+indexB++);
			// printf("C[%d] = %d\n",i,*(arrayC+i));
		}

	} else if (maxB == -1){
		// printf("maxC = %d\n",maxC);
		for (i = minC; i <= maxC; i++){
			*(arrayC+i) = *(arrayA+indexA++);
			// printf("C[%d] = %d\n",i,*(arrayC+i));
		}
	
	} else {
		// printf("maxC = %d\n",maxC);
		for (i = minC; i <= maxC; i++) {
			if (((*(arrayA+indexA) < *(arrayB+indexB)) && (indexA <= maxA)) || (indexB > maxB)) {
				*(arrayC+i) = *(arrayA+indexA++);
			} else if (((*(arrayB+indexB) <= *(arrayA+indexA)) && (indexB <= maxB)) || (indexA > maxA)) {
				*(arrayC+i) = *(arrayB+indexB++);
			}
			// printf("C[%d] = %d\n",i,*(arrayC+i));
		}
	}
}

int rank (int value, int* array, int sizeArray){
	int i = 0, cnt = 0;
	for (i=0; i<sizeArray; i++){
		if (*(array+i) <= value)
			cnt++;
	}

	return cnt;
}


void *pardo_rank(void *threadarg)
{
	thread_data1 *t1 = (thread_data1*) threadarg;  //typecast threadarg
	// printf("pardo_rank(%d): value = %d\tresult = %d\n",t1 -> id,t1 -> value,rank(t1 -> value, t1 -> array, t1 -> sizeArray));	
	*(t1 -> destination) = rank(t1 -> value, t1 -> array, t1 -> sizeArray);	
	
	pthread_exit(NULL);
}

void *pardo_seqmerge(void *threadarg)
{
	thread_data2 *t2 = (thread_data2*) threadarg;  //typecast threadarg
	// printf("pardo_seqmerge(%d): minA = %d\tmaxA = %d\tminB = %d\tmaxB = %d\tminC = %d\n",t2 -> id,t2->minA,t2->maxA,t2->minB,t2->maxB,t2->minC);
	seqmerge(t2->minA,t2->maxA,t2->minB,t2->maxB,t2->minC,t2->A,t2->B,t2->C);	
	
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

	printf("Joining %d threads \n",t);
	for(i=0; i<t; i++) {
		pthread_attr_destroy(&attr);
		rc = pthread_join(threads[i],NULL);
		handlerc(rc,"join",i);
	}
}