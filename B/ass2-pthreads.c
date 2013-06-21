#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_THREADS 3
#define MAX_INPUT_SIZE 64
#define NELEMS(x)  (sizeof(x) / sizeof(x[0])) // number of elements in an array

void seqmerge (int minA, int maxA, int minB, int maxB, int minC, int* arrayA, int* arrayB, int* arrayC);
int rank (int value, int* array, int sizeArray);

int main (int argc, char *argv[]) {
	int A[] = {28, 36, 40, 61, 68, 71,123, 149};
	int B[] = {2, 5, 18, 21, 24, 29, 31, 33, 34, 35, 47, 48, 49, 50, 52, 62, 66, 70, 73, 80, 88, 89, 114, 124, 125, 131, 143, 144, 145, 148, 155, 159};

	// int A[] = {1,3,9,15,16,21};
	// int B[] = {2,6,7,9,15,27,28};

	// int A[] = {1,3,9,15};
	// int B[] = {2,7};

	int sizeA = NELEMS(A);
	int sizeB = NELEMS(B);
	int sizeAB = sizeA+sizeB;

	int arrayAB[sizeAB];
	int resultA[sizeA];
	int resultB[sizeB];
	
	int AA[sizeA];
	int BB[sizeB];
	int AAi[sizeA];
	int BBi[sizeB];

	int C[sizeAB];
	int C_assigned[sizeAB];
	int cnt = 0;
	int minA = 0;
	int minB = 0;
	int minC = 0;
	int maxA = 0;
	int maxB = 0;

	int a = floor(log2(sizeA));
	int b = floor(log2(sizeB));
	AA[0] = 0;
	
	int i = 0;
	int j = 0;

	printf("STEP1:\na = %d\tb = %d\tsizeA = %d\tsizeB = %d\n",a,b,sizeA,sizeB);

	printf("\nSTEP2:\n");
	for (i = 1; i <= sizeA/a; i++){
		AA[i-1] = rank(A[i*a-1]-1,B,sizeB);
		AAi[i-1] = i*a-1;
		printf("rank(%d,B,%d)\t",A[i*a-1]-1,sizeB);
		printf("AAi[%d] = %d\tAA[%d] = %d\n",i,AAi[i-1],i,AA[i-1]);
	}
	for (j = 1; j <= sizeB/b; j++){
		BB[j-1] = rank(B[j*b-1],A,sizeA);
		BBi[j-1] = j*b;
		printf("rank(%d,A,%d)\t",B[j*b-1],sizeA);
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

		printf("indexA = %d\tindexB = %d\n",indexA,indexB);
		
		if (i == 0) {
			minA = 0;
			minB = 0;
			minC = 0;
		}
		
		
		if (((BB[indexB] <= AAi[indexA]) && (indexB <= sizeB/b-1)) || (indexA > sizeA/a-1)) {
			printf("maxA = BB[%d]\t",indexB);
			maxA = BB[indexB]-1;
		} else {
			printf("maxA = AAi[%d]\t",indexA);
			maxA = AAi[indexA]-1;
		}

		if (((BBi[indexB] <= AA[indexA]) && (indexB <= sizeB/b-1)) || (indexA > sizeA/a-1)) {
			printf("maxB = BBi[%d]\t",indexB);
			maxB = BBi[indexB++]-1;
		} else {
			printf("maxB = AA[%d]\t",indexA);
			maxB = AA[indexA++]-1;
		}

		if(i == a+b) { //last run
			maxA = sizeA-1;
			maxB = sizeB-1;
		}


		printf("minA = %d\tmaxA = %d\tminB = %d\tmaxB = %d\tminC = %d\tA\tB\tC\n\n",minA,maxA,minB,maxB,minC);
		seqmerge (minA,maxA,minB,maxB,minC,A,B,C);

	}

	

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

	return 0;
}

void seqmerge (int minA, int maxA, int minB, int maxB, int minC, int* arrayA, int* arrayB, int* arrayC){

	int indexA = minA;
	int indexB = minB;
	int maxC = minC + (maxA-minA) + (maxB-minB) + 1;
	int i = 0;

	printf("minA = %d\tminB = %d\tminC = %d\t",minA,minB,minC);
	if (maxA == -1) {
		printf("maxC = %d\n",maxC);
		for (i = minC; i <= maxC; i++) {
			*(arrayC+i) = *(arrayB+indexB++);
			printf("C[%d] = %d\n",i,*(arrayC+i));
		}

	} else if (maxB == -1){
		printf("maxC = %d\n",maxC);
		for (i = minC; i <= maxC; i++){
			*(arrayC+i) = *(arrayA+indexA++);
			printf("C[%d] = %d\n",i,*(arrayC+i));
		}
	
	} else {
		printf("maxC = %d\n",maxC);
		for (i = minC; i <= maxC; i++) {
			if (((*(arrayA+indexA) < *(arrayB+indexB)) && (indexA <= maxA)) || (indexB > maxB)) {
				*(arrayC+i) = *(arrayA+indexA++);
			} else if (((*(arrayB+indexB) <= *(arrayA+indexA)) && (indexB <= maxB)) || (indexA > maxA)) {
				*(arrayC+i) = *(arrayB+indexB++);
			}
			printf("C[%d] = %d\n",i,*(arrayC+i));
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
