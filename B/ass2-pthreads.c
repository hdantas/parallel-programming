#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_THREADS 3
#define MAX_INPUT_SIZE 64
#define NELEMS(x)  (sizeof(x) / sizeof(x[0])) // number of elements in an array


int rank (int value, int* array, int sizeArray);

int main (int argc, char *argv[]) {
	int arrayA[] = {28, 36, 40, 61, 68, 71,123, 149};
	int arrayB[] = {2, 5, 18, 21, 24, 29, 31, 33, 34, 35, 47, 48, 49, 50, 52, 62, 66, 70, 73, 80, 88, 89, 114, 124, 125, 131, 143, 144, 145, 148, 155, 159};
	int sizeA = NELEMS(arrayA);
	int sizeB = NELEMS(arrayB);
	int sizeAB = sizeA+sizeB;

	int arrayAB[sizeAB];
	int resultA[sizeA];
	int resultB[sizeB];
	
	int i = 0;
	for (i=0; i < sizeAB; i++){ // concatenate vectors
		if (i<sizeA)
			arrayAB[i]=arrayA[i];
		else
			arrayAB[i]=arrayB[i-sizeA];
	}

	printf("Rank(A:AB)\n");
	for (i=0; i < sizeA; i++){
		*(resultA + i) = rank (*(arrayA+i), arrayAB, sizeAB);
		printf("%d%s",*(resultA+i),(i==(sizeA-1))?"\n":", ");
	}

	printf("\nRank(B:AB)\n");
	for (i=0; i < sizeB; i++){
		*(resultB + i) = rank (*(arrayB+i), arrayAB, sizeAB);
		printf("%d%s",*(resultB+i),(i==(sizeB-1))?"\n":", ");
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