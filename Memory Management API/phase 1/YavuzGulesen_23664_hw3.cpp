#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <stdlib.h>
#include <queue>
#include <semaphore.h>
#include <time.h>
using namespace std;

#define NUM_THREADS 5
#define MEMORY_SIZE 1000

struct node
{
    int id;
    int size;
    
    node(int i, int s): id(i), size(s) {};
};

queue<node> myqueue; // shared que
pthread_mutex_t sharedLock = PTHREAD_MUTEX_INITIALIZER; // mutex
pthread_mutex_t screenLock = PTHREAD_MUTEX_INITIALIZER; // mutex
pthread_t server; // server thread handle
sem_t semlist[NUM_THREADS]; // thread semaphores

int thread_message[NUM_THREADS]; // thread memory information
char  memory[MEMORY_SIZE]; // memory size

void release_function(pthread_t* threadArray)
{
    //This function will be called
    //whenever the memory is no longer needed.
    //It will kill all the threads and deallocate all the data structures.
    while(myqueue.size() > 0) {
        myqueue.pop();
    }
    
    for(int i = 0; i < NUM_THREADS; i++) {
        pthread_cancel(threadArray[i]);
    }
    free(threadArray);
}

void my_malloc(int thread_id, int size)
{
    //This function will add the struct to the queue
    pthread_mutex_lock(&sharedLock);
    myqueue.push(node(thread_id, size));
    pthread_mutex_unlock(&sharedLock);
    //cout << "New queue size: " << myqueue.size() << endl;
}

void * server_function(void* id)
{
    //This function should grant or decline a thread depending on memory size.
    int index = 0;
    while(true) {
        if(myqueue.size() > 0) {
            //cout << "New task!" << endl;
            pthread_mutex_lock(&sharedLock);
            node frontNode = myqueue.front();
            myqueue.pop();
            //int value;
            //sem_getvalue(&semlist[n.id], &value);
            pthread_mutex_unlock(&sharedLock);
            //pthread_mutex_lock(&screenLock);
            //cout << "semvalue[" << n.id << "]: " << value << endl;
            //pthread_mutex_unlock(&screenLock);
            if(frontNode.size < MEMORY_SIZE - index) {
                pthread_mutex_lock(&sharedLock);
                thread_message[frontNode.id] = index;
                index += frontNode.size;
                sem_post(&semlist[frontNode.id]);
                pthread_mutex_unlock(&sharedLock);
                //pthread_mutex_lock(&screenLock);
                //cout << "Assigned " << n.size << " blocks to thread " << n.id << endl;
                //pthread_mutex_lock(&screenLock);
            }
            else {
                pthread_mutex_lock(&sharedLock);
                thread_message[frontNode.id] = -1;
                sem_post(&semlist[frontNode.id]);
                pthread_mutex_unlock(&sharedLock);
            }
        }
        else {
            sleep(0.05);
        }
    }
    return NULL;
}

void * thread_function(void* id)
{
    //This function will create a random size, and call my_malloc
   
    int _id = *(int*)id;
    int size;
	/*
    for(int i = 0; i < _id + 1; i++) {
        size = rand() % (MEMORY_SIZE/2);
		cout<<size<<endl;
    }*/
	 size = rand() % (MEMORY_SIZE/2);
	 //cout<<size<<endl;
    //pthread_mutex_lock(&screenLock);
    //cout << "Thread " << _id << " queued " << size << endl;
    //pthread_mutex_unlock(&screenLock);
    my_malloc(_id, size);
    //Block
    int value;
    sem_getvalue(&semlist[_id], &value);
    //pthread_mutex_lock(&sharedLock);
    //cout << "prewait sem: " << value << endl;
    //pthread_mutex_unlock(&sharedLock);
    sem_wait(&semlist[_id]);
    //pthread_mutex_lock(&screenLock);
    //cout << "Thread " << _id << " unblocked" << endl;
    //pthread_mutex_unlock(&screenLock);
    //Then fill the memory with 1's or give an error prompt
    int index = thread_message[_id];
    if(index != -1) {
        pthread_mutex_lock(&sharedLock);
        for(int i = 0; i < size; i++) {
            memory[index + i] = '1';
        }
        pthread_mutex_unlock(&sharedLock);
    }
    else {
        pthread_mutex_lock(&screenLock);
        cout << "Thread " << _id << ": Not enough memory" << endl;
        pthread_mutex_unlock(&screenLock);
    }
    return NULL;
}

void init()
{
    pthread_mutex_lock(&sharedLock);	//lock
    for(int i = 0; i < NUM_THREADS; i++) //initialize semaphores
    {sem_init(&semlist[i],0,0);}
    /*int value;
     for(int i = 0; i < NUM_THREADS; i++) //initialize semaphores
     {
     sem_getvalue(&semlist[i], &value);
     cout << "init[" << i << "]: " << value << endl;
     }
     cout << endl;*/
    for (int i = 0; i < MEMORY_SIZE; i++)	//initialize memory
    {char zero = '0'; memory[i] = zero;}
   	pthread_create(&server,NULL,server_function,NULL); //start server
    pthread_mutex_unlock(&sharedLock); //unlock
}

void dump_memory()
{
    // You need to print the whole memory array here.
    //pthread_mutex_lock(&screenLock);
    cout << "Memory Dump:" << endl;
    for(int i = 0; i < MEMORY_SIZE; i++) {
        cout << memory[i];
    }
    cout << endl;
    //pthread_mutex_unlock(&screenLock);
}

int main (int argc, char *argv[])
{
	 srand(time(NULL));
	//You need to create a thread ID array here
    pthread_t* threadArray = (pthread_t *)calloc(NUM_THREADS, sizeof(pthread_t));
    //cout << "init queue size: " << myqueue.size() << endl;
    
    init();	// call init
    
    //You need to create threads with using thread ID array, using pthread_create()
    int* index = (int *)calloc(NUM_THREADS, sizeof(int));
    for(int i = 0; i < NUM_THREADS; i++) {
        index[i] = i;
        if(pthread_create(&threadArray[i], NULL, thread_function, &index[i]) != 0) {
            pthread_mutex_lock(&screenLock);
            cout << "Error creating thread: " << i << endl;
            pthread_mutex_unlock(&screenLock);
        }
    }
    
    //You need to join the threads
    for(int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threadArray[i], NULL);
    }
    dump_memory(); // this will print out the memory
    printf("\nMemory Indexes:\n" );
    for (int i = 0; i < NUM_THREADS; i++)
    {
        //pthread_mutex_lock(&screenLock);
        printf("[%d]" ,thread_message[i]); // this will print out the memory indexes
        //pthread_mutex_unlock(&screenLock);
    }
    printf("\nTerminating...\n");
}
