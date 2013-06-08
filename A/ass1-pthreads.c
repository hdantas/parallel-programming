#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_THREADS 4
#define MAX_INPUT_SIZE 32

typedef enum { false, true } bool; //fake booleans for C
/********************************************************************
Algorithm:

Create a vector of thread_data with the size: input_length-1. Each position represents a different "task".
Assign a task to each thread while there are threads available and task have their dependencies ready.
Lock data while a thread is working on it. At the end mark struct instance as done and unlock it.
Keep trying to assign tasks to threads while there are uncomplete tasks.
Done! Print result and terminate.

********************************************************************/

int* prefixMinima (int* left, int* right, int sizeleft, int sizeright);
int* suffixMinima (int* left, int* right, int sizeleft, int sizeright);
//void *ComputeMinima(void *threadarg);

struct thread_data{
	int* left;
	int* right;
	int* pm;
	int* sm;
	int leftsize;
	int rightsize;
	bool complete;
};

struct thread_data thread_data_array[MAX_INPUT_SIZE-1];


void *ComputeMinima(void *threadarg)
{
	int i;
	struct thread_data *my_data;
	sleep(1);
	my_data = (struct thread_data *) threadarg;
	

	my_data->pm = prefixMinima (my_data->left,my_data->right,my_data->leftsize,my_data->rightsize);
	my_data->sm = suffixMinima (my_data->left,my_data->right,my_data->leftsize,my_data->rightsize);
	
	printf("\nStarting:\t");
	printf("\npreffixMinima:\t");
	for (i = 0; i<6; i++)
		printf("%d%c ",*(my_data->pm+i),(i==5)?'\n':',');

	printf("\bsuffixMinima:\t");
	for (i = 0; i<6; i++)
		printf("%d%c ",*(my_data->sm+i),(i==5)?'\n':',');

	pthread_exit(NULL);
}



int main (int argc, char *argv[]) {
	int array[] = {2,1,3,4,5,6};
	// int array[] = {58,89,32,73,131,156,30,29,141,37,133,151,88,53,122,126,131,49,130,115,16,83,40,145,10,112,20,147,14,104,111,92};
	pthread_t threads[NUM_THREADS];
	pthread_attr_t attr;

	int rc, i;
	long t;
	void* status;
	
	// int length = sizeof(array)/sizeof(int);
	// int i;
	
	// printf("index:\t\t");
	// for (i=1;i<=length;i++) {
	// 	if (i<10)
	// 		printf("%d  ",i);
	// 	else
	// 		printf("%d ",i);
	// }

	// pm1 = prefixMinima (&array[0],&array[1],1,1);
	// sm1 = suffixMinima (&array[0],&array[1],1,1);

	
	/* Initialize and set thread detached attribute */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);



	for(t=0;t<NUM_THREADS;t++) {
		
		thread_data_array[t].left=&array[2*t];
		thread_data_array[t].right=&array[2*t+1];
		thread_data_array[t].leftsize=1;
		thread_data_array[t].rightsize=1;

		printf("\nCreating thread %ld\n", t);
		rc = pthread_create(&threads[t], &attr, ComputeMinima, (void *) &thread_data_array[t]);
		if (rc) {
		  printf("ERROR; return code from pthread_create() is %d\n", rc);
		  exit(-1);
		}
	}


	/* Free attribute and wait for the other threads */
	pthread_attr_destroy(&attr);
	for(t=0; t<NUM_THREADS; t++) {
		rc = pthread_join(threads[t], &status);
		if (rc) {
			printf("ERROR; return code from pthread_join() is %d\n", rc);
			exit(-1);
		}
		printf("\bMain: completed join with thread %ld having a status of %ld\n",t,(long)status);
	}


	printf("Main preffixMinima:\t");
	for (i = 0; i<6; i++)
		printf("%d%c ",*(thread_data_array[NUM_THREADS-1].pm+i),(i==5)?'\n':',');

	printf("\bMain suffixMinima:\t");
	for (i = 0; i<6; i++)
		printf("%d%c ",*(thread_data_array[NUM_THREADS-1].sm+i),(i==5)?'\n':',');

	printf("\bMain: program completed. Exiting.\n");
	/* Last thing that main() should do */
	pthread_exit(NULL);

}


int* prefixMinima (int* left, int* right, int sizeleft, int sizeright) {
	int lastleft = *(left+sizeleft-1);
	int firstright = *right;
	int* outputarray = malloc((sizeleft+sizeright)*sizeof(int));
	int i;

	memcpy(outputarray,left,sizeleft*sizeof(int));
	if (lastleft >= firstright) //concatenate arrays
		memcpy(outputarray+sizeleft,right,sizeright*sizeof(int)); //append right array to output
	else { //replace the complete right array with lastleft
		for (i=0; i<sizeright; i++)
			*(outputarray+sizeleft+i)=lastleft;
	}

	return outputarray;
}

int* suffixMinima (int* left, int* right, int sizeleft, int sizeright) {
	int lastleft = *(left+sizeleft-1);
	int firstright = *right;
	int* outputarray = malloc((sizeleft+sizeright)*sizeof(int));
	int i;

	memcpy(outputarray+sizeleft,right,sizeright*sizeof(int));
	if (lastleft <= firstright) //concatenate arrays
		memcpy(outputarray,left,sizeleft*sizeof(int)); //preppend left array to output
	else { //replace the complete left array with firstright
		for (i=0; i<sizeleft; i++)
			*(outputarray+i)=firstright;
	}

	return outputarray;
}