#include <stdio.h>

void prefixMinima (int* array, int length);
int* suffixMinima (int* array, int length);

int main (int argc, char *argv[]) {
	int length = 32;
	int array[] = {58,89,32,73,131,156,30,29,141,37,133,151,88,53,122,126,131,49,130,115,16,83,40,145,10,112,20,147,14,104,111,92};
	printf("preffixMinima:\t");
	prefixMinima (array,length);
	printf("\nsuffixMinima:\t");
	suffixMinima (array,length);
	printf("\n");
	return 0;
}


void prefixMinima (int* array, int length) {
	int* ptr = array;
	int currentmin = *ptr;
	int i;
	for (i=0; i<length; i++) {
		if (*(ptr+i) < currentmin)
			currentmin = *(ptr+i);	
		printf("%d ",currentmin);
	}

}

int* suffixMinima (int* array, int length) {
	int currentmin = *array;
	int newArray[length];
	int* ptr1 = array;
	int* ptr2 = array;
	int i,j;
	for (i=0; i<length; i++) {
		currentmin = *(ptr1+i);
		for (j=i; j<length; j++) {	
			if (*(ptr2+j) < currentmin)
				currentmin = *(ptr2+j);	
		}
		printf("%d ",currentmin);
	}	
	
}