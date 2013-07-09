#include <math.h>
#include <stdio.h>

#define NELEMS(x)  (sizeof(x) / sizeof(x[0])) // number of elements in an array
#define NSIZE 16

void pointerJumping (int* array, int length);

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
	int S[NSIZE] = {0};
	int D[NSIZE] = {0};
	int newS[NSIZE] = {0};
	int newD[NSIZE] = {0};
	int i,j;
	
	for (i=0; i<=length; i++) {
		S[i] = P[i];

		if (0 != S[i])
			D[i] = 1;
		else
			D[i] = 0;
	}

	for (j=0; j<=log2(length); j++) {	
		printf("Iteration %d\n",j+1);
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
		for (i=1; i<=length; i++) {
			printf("D[%d]=%d,%s",i,D[i],(D[i]>9)?" ":"  ");
			S[i] = newS[i];
			D[i] = newD[i];
		}
		printf("\n\n");
	}
}
