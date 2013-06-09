#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_THREADS 2
#define MAX_INPUT_SIZE 32
#define NELEMS(x)  (sizeof(x) / sizeof(x[0])) // number of elements in an array


typedef enum { false, true } bool; //fake booleans for C
/********************************************************************
Algorithm:

Create a vector of thread_data with the size: input_length-1. Each position represents a different "task".
Assign a task to each thread while there are threads available and task have their dependencies ready.
Lock data while a thread is working on it. At the end mark struct instance as done and unlock it.
Keep trying to assign tasks to threads while there are uncomplete tasks.
Done! Print result and terminate.

********************************************************************/
struct thread_data{
	struct thread_data* left;
	struct thread_data* right;
	int leftsize;
	int rightsize;
	int* pm;
	int* sm;
};

struct thread_data thread_data_array[NUM_THREADS];

struct thread_data* queue1[MAX_INPUT_SIZE];
struct thread_data* queue2[MAX_INPUT_SIZE];



void handlerc(int rc, char* string, long t);
int* prefixMinima (int* left, int* right, int sizeleft, int sizeright);
int* suffixMinima (int* left, int* right, int sizeleft, int sizeright);
void pushToQueue (struct thread_data** queue, long* counter, struct thread_data *value);
struct thread_data* pullFromQueue (struct thread_data** queue,long *counter);
//void *ComputeMinima(void *threadarg);




void *ComputeMinima(void *threadarg)
{
	return (void *) NULL;
	/*int i;
	struct thread_data *my_data;
	sleep(1);
	my_data = (struct thread_data *) threadarg;
	

	my_data->pm = prefixMinima (my_data->left,my_data->right,my_data->leftsize,my_data->rightsize);//CHANGE THIS
	my_data->sm = suffixMinima (my_data->left,my_data->right,my_data->leftsize,my_data->rightsize);
	
	printf("\nStarting:\t");
	printf("\npreffixMinima:\t");
	for (i = 0; i<6; i++)
		printf("%d%c ",*(my_data->pm+i),(i==5)?'\n':','); // the weird thing in the printf is to avoid having comma in the last cycle

	printf("\bsuffixMinima:\t");
	for (i = 0; i<6; i++)
		printf("%d%c ",*(my_data->sm+i),(i==5)?'\n':',');

	return (void *) my_data;*/
}



int main (int argc, char *argv[]) {

	int array[] = {1,2,3,4,5,6,7,8,9};
	// int array[] = {58,89,32,73,131,156,30,29,141,37,133,151,88,53,122,126,131,49,130,115,16,83,40,145,10,112,20,147,14,104,111,92};
	pthread_t threads[NUM_THREADS];
	pthread_attr_t attr;

	int rc, i, j, k;
	long t1 = 0, t2 = 0;
	void* status;
	int arraysize = NELEMS(array);
	int heightTree = ceil(log2(arraysize));
	int elementsperlevel[heightTree];

	struct thread_data **thislevelqueue_ptr = queue1;
	struct thread_data **nextlevelqueue_ptr = queue2;
	volatile long thislevelqueue_cnt = 0; // volatile to prevent infinite loops later
	volatile long nextlevelqueue_cnt = 0; // volatile to prevent infinite loops later


	j = arraysize;
	for (k=0;k<heightTree;k++){
		elementsperlevel[k]=j;

		if (j%2 == 0)
			j /= 2;
		else
			j = floor(j/2)+1;
	}

	for (i=0;i<NELEMS(elementsperlevel);i++){
		printf("elementsperlevel[%d]:%d\n",i,elementsperlevel[i]);
	}
	
	/* Initialize and set thread joinable attribute */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	
	int elementbeingprocessed = 0;
	int processedelements = 0;

	
	// FILL this_level_queue FOR THE 1ST TIME, CONVERT ARRAY VALUES TO STRUCT DATA


	for (k=0;k<heightTree;k++){
		while(thislevelqueue_cnt < elementsperlevel[k]){
			
			for (t1 = 0; t1 < NUM_THREADS; t1++){ //spawn new threads
				if (thislevelqueue_cnt >= (elementsperlevel[k] - 1)){ // in case there are not enough unprocess inputs
					t1--; //"interrupt" this thread
					printf("\nBREAK:T1=%ld\tT2=%ld\t\tthislevelqueue_cnt:%ld\tnextlevelqueue_cnt:%ld\n",t1,t2,thislevelqueue_cnt,nextlevelqueue_cnt);	
					break;
				}
				//thread_data_array[t1].left= pullFromQueue(&thislevelqueue_ptr,&thislevelqueue_cnt);
				thislevelqueue_cnt++;
				//thread_data_array[t1].right= pullFromQueue(&thislevelqueue_ptr,&thislevelqueue_cnt);
				thislevelqueue_cnt++;
				printf("Creating thread %ld in level%d.\n",t1,k);
				/*rc = pthread_create(&threads[t1], &attr, ComputeMinima, (void *) &thread_data_array[t1]);
				handlerc(rc,"create",t1);*/
				
				printf("SPAWN:T1=%ld\tT2=%ld\t\tthislevelqueue_cnt:%ld\tnextlevelqueue_cnt:%ld\n",t1,t2,thislevelqueue_cnt,nextlevelqueue_cnt);
			}
			printf("\n");
			for (t2 = 0; t2 < t1; t2++){ //merge spawned threads
				printf("MERGE:T1=%ld\tT2=%ld\t\tthislevelqueue_cnt:%ld\tnextlevelqueue_cnt:%ld\n",t1,t2,thislevelqueue_cnt,nextlevelqueue_cnt);
				printf("Joining thread %ld in level%d.\n",t2,k);
				/*rc = pthread_join(threads[t2], &status);
				handlerc(rc,"join",t2);*/
				//pushToQueue(&nextlevelqueue_ptr,&nextlevelqueue_cnt,(struct thread_data *)status); //push obtained results to queue
				nextlevelqueue_cnt++;
				printf("MERGE:T1=%ld\tT2=%ld\t\tthislevelqueue_cnt:%ld\tnextlevelqueue_cnt:%ld\n",t1,t2,thislevelqueue_cnt,nextlevelqueue_cnt);
			}

			if (thislevelqueue_cnt == elementsperlevel[k] - 1){ //in case there is a unpaired element at the end of the input array
				//thread_data_array[0].left= pullFromQueue(&thislevelqueue_ptr,&thislevelqueue_cnt);
				thislevelqueue_cnt++;
				//pushToQueue(&nextlevelqueue_ptr,&nextlevelqueue_cnt,&thread_data_array[0]); //push unpaired element to queue
				nextlevelqueue_cnt++;
				printf("\nREMAINDER:T1=%ld\tT2=%ld\t\tthislevelqueue_cnt:%ld\tnextlevelqueue_cnt:%ld\n",t1,t2,thislevelqueue_cnt,nextlevelqueue_cnt);
			}

			printf("\n######\n\n");
		}

		printf("Resetting variables\n");
		//next level queue becomes current level
		thislevelqueue_ptr = nextlevelqueue_ptr;
		thislevelqueue_cnt = 0;
		nextlevelqueue_cnt = 0;
		
	}


	// Free attribute
	pthread_attr_destroy(&attr);

	/*printf("Main preffixMinima:\t");
	for (i = 0; i<6; i++)
		printf("%d%c ",*(thread_data_array[NUM_THREADS-1].pm+i),(i==5)?'\n':',');

	printf("\bMain suffixMinima:\t");
	for (i = 0; i<6; i++)
		printf("%d%c ",*(thread_data_array[NUM_THREADS-1].sm+i),(i==5)?'\n':',');*/

	printf("\bMain: program completed. Exiting.\n");
	// Last thing that main() should do 
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

void handlerc(int rc,char* string, long t){
	if (rc) {
	  printf("ERROR; return code from pthread_%s(%ld) is %d\n",string,t,rc);
	  exit(-1);
	}
}

void pushToQueue (struct thread_data** queue, long* counter, struct thread_data *value){
	long cnt = *counter;
	*(queue + cnt) = value;
	*counter = cnt + 1;
}

struct thread_data* pullFromQueue (struct thread_data** queue,long *counter){
	long cnt = *counter;
	*counter = cnt + 1;
	return *(queue + cnt);
}

