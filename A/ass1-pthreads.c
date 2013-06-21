#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_THREADS 3
#define MAX_INPUT_SIZE 64
#define NELEMS(x)  (sizeof(x) / sizeof(x[0])) // number of elements in an array


/********************************************************************
Algorithm:

Create a vector of thread_data with the size: input_length-1. Each position represents a different "task".
Assign a task to each thread while there are threads available and task have their dependencies ready.
Lock data while a thread is working on it. At the end mark struct instance as done and unlock it.
Keep trying to assign tasks to threads while there are uncomplete tasks.
Done! Print result and terminate.

********************************************************************/
typedef struct thread_data{
	struct thread_data* leftstruct;
	struct thread_data* rightstruct;
	int size;
	int* pm;
	int* sm;
} thread_data;




void handlerc(int rc, char* string, long t);
int* prefixMinima (thread_data* leftstruct, thread_data* rightstruct, int leftsize, int rightsize);
int* suffixMinima (thread_data* leftstruct, thread_data* rightstruct, int leftsize, int rightsize);
void pushToQueue (thread_data** queue, volatile long* counter, thread_data *value);
thread_data* pullFromQueue (thread_data** queue, volatile long *counter);
void *ComputeMinima(void *threadarg);




int main (int argc, char *argv[]) {

	// int array[] = {58,89,32,73,131,156,30,29};
	int array[] = {58,89,32,73,131,156,30,29,141,37,133,151,88,53,122,126,131,49,130,115,16,83,40,145,10,112,20,147,14,104,111,92};
	pthread_t threads[NUM_THREADS];
	pthread_attr_t attr;

	int rc, i, j, k;
	long t1 = 0, t2 = 0;
	void* status;
	int arraysize = NELEMS(array);
	int heightTree = ceil(log2(arraysize));
	int elementsperlevel[heightTree];

	thread_data thread_data_instances[NUM_THREADS] = {{}}; //let compiler initialize all struct instances to zeros or NULL. ugly :S
	thread_data* thread_data_array[NUM_THREADS];
	

	thread_data* queue1[MAX_INPUT_SIZE] = {0};
	thread_data* queue2[MAX_INPUT_SIZE] = {0};
	thread_data* queueZEROS[MAX_INPUT_SIZE] = {0};



	thread_data **thislevelqueue_ptr = queue1;
	thread_data **nextlevelqueue_ptr = queue2;

	volatile long thislevelqueue_cnt = 0; // volatile to prevent infinite loops later
	volatile long nextlevelqueue_cnt = 0; // volatile to prevent infinite loops later

	for (i=0;i<NUM_THREADS;i++){ //thread_data_array contains pointers to each instance in the thread_data_instances
		thread_data_array[i] = &(thread_data_instances[i]);
	}


	j = arraysize;
	for (k = 0; k < heightTree; k++){
		elementsperlevel[k]=j;

		if (j%2 == 0)
			j /= 2;
		else
			j = floor(j/2)+1;
	}

	// for (i = 0; i < NELEMS(elementsperlevel); i++){
	// 	printf("elementsperlevel[%d]:%d\n",i,elementsperlevel[i]);
	// }
	
	/* Initialize and set thread joinable attribute */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	
	thread_data* tmp_struct_ptr;

	// initialize queue with input array
	for (i = 0; i < elementsperlevel[0]; i++){
		tmp_struct_ptr = (thread_data* ) malloc(sizeof(*tmp_struct_ptr)); //allocate space for a new thread_data struct
		/*tmp_struct.leftstruct = NULL;
		tmp_struct.rightstruct = NULL;*/
		*tmp_struct_ptr = (thread_data) {.leftstruct=NULL,.rightstruct=NULL,.size = 1,.pm = array + i,.sm = array + i};
		pushToQueue(thislevelqueue_ptr,&thislevelqueue_cnt,tmp_struct_ptr);
	}
	free(tmp_struct_ptr);
	thislevelqueue_cnt = 0;

	for (k = 0; k < heightTree; k++){
		while(thislevelqueue_cnt < elementsperlevel[k]){
			
			for (t1 = 0; t1 < NUM_THREADS; t1++){ //spawn new threads
				if (thislevelqueue_cnt >= (elementsperlevel[k] - 1)){ // in case there are not enough unprocess inputs
					// printf("\nBREAK:T1=%ld\t\tthislevelqueue_cnt:%ld\tnextlevelqueue_cnt:%ld\n",t1,thislevelqueue_cnt,nextlevelqueue_cnt);	
					break;
				}
				thread_data_array[t1]->leftstruct= pullFromQueue(thislevelqueue_ptr,&thislevelqueue_cnt);
				thread_data_array[t1]->rightstruct= pullFromQueue(thislevelqueue_ptr,&thislevelqueue_cnt);
				// printf("\tCreating thread %ld in level%d.\t",t1,k);
				
				rc = pthread_create(&threads[t1], &attr, ComputeMinima, (void *) thread_data_array[t1]);
				handlerc(rc,"create",t1);
				
				// printf("SPAWN:T1=%ld\t\tthislevelqueue_cnt:%ld\tnextlevelqueue_cnt:%ld\n",t1,thislevelqueue_cnt,nextlevelqueue_cnt);
			}

			// printf("T1:%ld\tT2:%ld\n",t1,t2);
			
			for (t2 = 0; t2 < t1; t2++){ //merge spawned threads
				// printf("\tJoining thread %ld in level%d.\t",t2,k);
				
				rc = pthread_join(threads[t2], &status);
				handlerc(rc,"join",t2);
				
				pushToQueue(nextlevelqueue_ptr,&nextlevelqueue_cnt,(thread_data *)status); //push obtained results to queue
				// printf("MERGE:T2=%ld\t\tthislevelqueue_cnt:%ld\tnextlevelqueue_cnt:%ld\n",t2,thislevelqueue_cnt,nextlevelqueue_cnt);
			}

			if (thislevelqueue_cnt == elementsperlevel[k] - 1){ //in case there is a unpaired element at the end of the input array
				thread_data_array[0]= pullFromQueue(thislevelqueue_ptr,&thislevelqueue_cnt);
				pushToQueue(nextlevelqueue_ptr,&nextlevelqueue_cnt,thread_data_array[0]); //push unpaired element to queue
				// printf("\nREMAINDER:T1=%ld\tT2=%ld\t\tthislevelqueue_cnt:%ld\tnextlevelqueue_cnt:%ld\n",t1,t2,thislevelqueue_cnt,nextlevelqueue_cnt);
			}
			// printf("\n");

		}

		// printf("\n######\n\n");
		//next level queue becomes current level
		thislevelqueue_ptr = nextlevelqueue_ptr;
		nextlevelqueue_ptr = queueZEROS;
		
		thislevelqueue_cnt = 0;
		nextlevelqueue_cnt = 0;
		
	}


	// Free attribute
	pthread_attr_destroy(&attr);


	printf("\ni:\t\t\t");
	for (i = 1; i<=arraysize; i++)
		if (i<10)
			printf("0%d%c ",i,(i==arraysize)?'\n':',');
		else
			printf("%d%c ",i,(i==arraysize)?'\n':',');

	printf("\bMain preffixMinima:\t");
	for (i = 0; i<arraysize; i++)
		printf("%d%c ",*(nextlevelqueue_ptr[0]->pm+i),(i==(arraysize-1))?'\n':',');

	printf("\bMain suffixMinima:\t");
	for (i = 0; i<arraysize; i++)
		printf("%d%c ",*(nextlevelqueue_ptr[0]->sm+i),(i==(arraysize-1))?'\n':',');

	printf("\bMain: program completed. Exiting.\n");
	
	return(0);

}


int* prefixMinima (thread_data* leftstruct,thread_data* rightstruct, int sizeleft, int sizeright) {
	int* left = leftstruct->pm;
	int* right = rightstruct->pm;

	int min;
	int lastleft = *(left+sizeleft-1);
	int firstright = *right;
	int* outputarray = malloc((sizeleft+sizeright)*sizeof(int));
	int i;
	memcpy(outputarray,left,sizeleft*sizeof(int));
	if (lastleft >= firstright) //concatenate arrays
		memcpy(outputarray+sizeleft,right,sizeright*sizeof(int)); //append right array to output
	else {
		min = lastleft;
		for (i=0; i<sizeright; i++) {
			if (*(right+i) < min)
				min = *(right+i);

			*(outputarray+sizeleft+i)=min;
		}
	}

	return outputarray;
}

int* suffixMinima (thread_data* leftstruct, thread_data* rightstruct, int sizeleft, int sizeright) {
	int* left = leftstruct->sm;
	int* right = rightstruct->sm;

	int min;
	int lastleft = *(left+sizeleft-1);
	int firstright = *right;
	int* outputarray = malloc((sizeleft+sizeright)*sizeof(int));
	int i;
	memcpy(outputarray+sizeleft,right,sizeright*sizeof(int));
	if (lastleft <= firstright) //concatenate arrays
		memcpy(outputarray,left,sizeleft*sizeof(int)); //preppend left array to output
	else {
		min = firstright;
		for (i=sizeleft-1; i>=0; i--) {
			if(*(left+i) < min)
				min = *(left+i);

			*(outputarray+i)=min;
		}
	}

	return outputarray;
}

void handlerc(int rc,char* string, long t){
	if (rc) {
	  printf("ERROR; return code from pthread_%s(%ld) is %d\n",string,t,rc);
	  exit(-1);
	}
}

void pushToQueue (thread_data** queue, volatile long* counter, thread_data *value){
	*(queue + *counter) = value;
	*counter = *counter + 1;
}

thread_data* pullFromQueue (thread_data** queue, volatile long *counter){
	*counter = *counter + 1;
	return *(queue + *counter - 1);
}


void *ComputeMinima(void *threadarg)
{
	int tmp_size = 0;
	int* tmp_pm;
	int* tmp_sm;

	thread_data *new_threadarg = (thread_data*) threadarg;  //typecast threadarg
	thread_data *new_struct;
	new_struct = (thread_data*) malloc(sizeof(*new_struct)); //allocate space for a new thread_data struct
	 
	
	tmp_pm = prefixMinima(new_threadarg->leftstruct,new_threadarg->rightstruct,new_threadarg->leftstruct->size,new_threadarg->rightstruct->size);
	tmp_sm = suffixMinima(new_threadarg->leftstruct,new_threadarg->rightstruct,new_threadarg->leftstruct->size,new_threadarg->rightstruct->size);
	tmp_size = (new_threadarg->leftstruct->size+new_threadarg->rightstruct->size);


	*new_struct = (thread_data) {.leftstruct=new_threadarg->leftstruct,.rightstruct=new_threadarg->rightstruct,.size = tmp_size,.pm = tmp_pm,.sm = tmp_sm};

	// printf("\t\tpreffixMinima:\t");
	// for (i = 0; i < tmp_size; i++)
	// 	printf("%d%c ",*(new_struct->pm+i),(i==(tmp_size-1))?'\n':','); // the weird thing in the printf is to avoid having comma in the last cycle

	// printf("\t\tsuffixMinima:\t");
	// for (i = 0; i< tmp_size; i++)
	// 	printf("%d%c ",*(new_struct->sm+i),(i==(tmp_size-1))?'\n':',');
	// printf("new_struct=%p\n",new_struct);
	return (void *) new_struct;
}