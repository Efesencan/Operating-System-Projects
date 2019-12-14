#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <stdlib.h>
#include <queue>
#include <semaphore.h>
#include <list>
#include <iterator>
using namespace std;

#define NUM_THREADS 5
#define MEMORY_SIZE 10

struct link_node{    // struct for a linked list node
	int id,size,index;
};

struct node
{
	int id;
	int size;
};

list <link_node> memory_list;
int isFinished = 1;
int index = 0;
queue<node> myqueue; // shared que
pthread_mutex_t sharedLock = PTHREAD_MUTEX_INITIALIZER; // mutex
pthread_t server; // server thread handle
sem_t semlist[NUM_THREADS]; // thread semaphores
int element_size = 0;
int thread_message[NUM_THREADS]; // thread memory information
char  memory[MEMORY_SIZE]; // memory size



void release_function()
{
	while(!myqueue.empty()){
		myqueue.pop();
	}
	for(int i = 0; i<MEMORY_SIZE; i++){
		memory[i] = 'X';
	}
	for(int j = 0; j<NUM_THREADS; j++){
		sem_destroy(&semlist[j]);
	}
}

void dump_memory() // to print the memory array and linked list
{
 	list <link_node> :: iterator it;
	int count = 0;
  for(it = memory_list.begin(); it != memory_list.end(); ++it){
		count++;
		cout<<"["<<it->id<<"]["<<it->size<<"]["<<it->index<<"]";
		if(count != memory_list.size()){
			cout<<"----";
		}
	}
	cout<<endl;
 cout<<"Memory Dump:"<<endl;
 for(int i = 0; i<MEMORY_SIZE; i++){
   cout<<memory[i];
 }
 cout<<endl;
 std::cout << "****************************" << '\n';
// pthread_mutex_unlock(&sharedLock);

}

void use_mem(){ // wait 1-5 seconds
	int random = (rand() % (5)) + 1;
	sleep(random);
}

void * free_mem(int t_id){ // when thread is done update the linked list and memory array
	pthread_mutex_lock(&sharedLock);
	list <link_node> :: iterator it,second,temp,prev,next,temp2;
	for(it = memory_list.begin(); it != memory_list.end(); ++it){
		if(t_id == it->id){
			for(int j = 0; j<it->size; j++){
				memory[j + it->index] = 'X';
			}
			//cout<<"To be deleted: "<<it->id<<" "<<it->size<<" "<<it->index<<endl;
			if(it->id == memory_list.front().id){ // headsem
				second = it;
				if((++second)->id == -1){ // if head's next is a hole
					second->size += it->size;
					second->index = 0;
					memory_list.erase(it);
				}
				else{
					it->id = -1;
				}
			}
			else if(it->id == memory_list.back().id){ // If a am in the tail of a list
				prev= it;
				prev--;
				if(prev->id == -1){ // if tail's previous is hole
					prev->size += it->size;
					memory_list.erase(it);
				}
				else{
						it->id = -1;
					}
			}
			else{ // I am in the middle of linked list (nor tail nor head)
				prev = it;
				next = it;
				next++;
				prev--;
				if(prev->id != -1 && next->id != -1){ // left and right is not a tail
					it->id = -1;
				}
				else if(prev->id == -1 && next->id != -1){ // left is hole right is not
					prev->size += it->size;
					memory_list.erase(it);
				}
				else if(prev->id != -1 && next->id == -1){ // left is not hole right is hole
					next->size += it->size;
					next->index = it->index;
					memory_list.erase(it);
				}
				else{ // both left and right is a hole
					prev->size = prev->size + it->size + next->size;
					memory_list.erase(it);
					memory_list.erase(next);
				}
			}
		 	break;
		}
	}
	cout<<"I am in the free mem"<<endl;
	dump_memory();
	pthread_mutex_unlock(&sharedLock);
}

bool my_malloc(int thread_id, int size)
{
	//This function will add the struct to the queue
  pthread_mutex_lock(&sharedLock);
  node data;
  data.id = thread_id;
  data.size = size;
  myqueue.push(data);
	pthread_mutex_unlock(&sharedLock);
	sem_wait(&semlist[thread_id]);
		//element_size++;
		list <link_node> :: iterator it,second;
		 for(it = memory_list.begin(); it != memory_list.end(); ++it){ // check if we could insert the node
			 if(it->id == thread_id){ // this means that we inserted
				 return true;
			 }
		 }
		 return false;
}

void * server_function(void *)
{
  int left_mem = MEMORY_SIZE;
  while(isFinished){
    if(!myqueue.empty()){ // if queue is not empty keep popping element
			pthread_mutex_lock(&sharedLock);
      node front_val = myqueue.front();
      int mem_size = front_val.size;
      int t_id = front_val.id;
      myqueue.pop();
			list <link_node> :: iterator it;
			link_node my_node;
			my_node.id = t_id;
			my_node.size = mem_size;
			int index = 0;
			int counter = 0;
			element_size = memory_list.size();
			char ch;
			for(it = memory_list.begin(); it != memory_list.end(); ++it){
				if(it->id == -1 && mem_size <= it->size){ // check if we could insert that element to the linked list
					my_node.index = index;
					memory_list.insert(it,my_node);

					it->size -= my_node.size;
					it->index += my_node.size;
					if(it->size == 0){
						//cout<<it->id<<endl;
						memory_list.erase(it);
						//cout<<it->id<<endl;
					}
					int start_index = my_node.index;
					int length = my_node.size;
					ch = '0' + my_node.id;

					for(int i = 0; i<length; i++){ // update the memory array
						memory[start_index + i] = ch;
					}
					dump_memory();
					break;

				}else{
					index += it->size;
					counter += it->size;
				}

			}
      sem_post(&semlist[t_id]); // activate my malloc
			pthread_mutex_unlock(&sharedLock);
    }
  }
	//This function should grant or decline a thread depending on memory size.
}

void * thread_function(void * id)
{
  int * idPointer = (int *) id;
	while(isFinished){
		char ch = '0' + *idPointer; // convert int to char
	  int memory_size = (((rand() % (MEMORY_SIZE)%3))+1); // 1 veya 1/3 of the memory size
		bool isAdded = my_malloc(*idPointer,memory_size);
	  if(isAdded){ // if the node is inserted than we can call use mem and free mem function
			use_mem();
			free_mem(*idPointer);
		}
	}
	//cout<<"terminated thread func"<<*idPointer<<endl;
}

void init()
{
	pthread_mutex_lock(&sharedLock);	//lock
	for(int i = 0; i < NUM_THREADS; i++) //initialize semaphores
	{sem_init(&semlist[i],0,0);}
	for (int i = 0; i < MEMORY_SIZE; i++)	//initialize memory
  	{char zero = 'X'; memory[i] = zero;}
   	pthread_create(&server,NULL,server_function,NULL); //start server
	pthread_mutex_unlock(&sharedLock); //unlock
}


int main (int argc, char *argv[])
 {
   srand(7);
	 link_node my_node;
	 my_node.id = -1;
	 my_node.size = MEMORY_SIZE;
	 my_node.index = 0;
	 memory_list.push_back(my_node);
	 element_size = memory_list.size();
 	//You need to create a thread ID array here
  pthread_t threadArray[NUM_THREADS];
  int thread_ID[NUM_THREADS];

  for (int i = 0; i<NUM_THREADS; i++) {
    thread_ID[i] = i;
    pthread_t memoryThread;
    threadArray[i] = memoryThread;
    /* code */
  }

 	init();	// call init

 	//You need to create threads with using thread ID array, using pthread_create()
  for(int i = 0; i<NUM_THREADS; i++){
    pthread_create( &threadArray[i], NULL, thread_function, (void*) &thread_ID[i]);
  }
  sleep(10);
	isFinished = 0;

  for (int i = 0; i< NUM_THREADS; i++) {
    pthread_join(threadArray[i], NULL);
  }
  release_function();

 	printf("\nTerminating...\n");
 }
