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
//-std=c++11
#define NUM_THREADS 5
#define MEMORY_SIZE 30
#define THREAD_SLEEP 5
#define RUNTIME_DURATION 10

struct malloc_req
{
    int id;
    int size;
    
    malloc_req(int i, int s): id(i), size(s) {};
};

class LinkedList
{
    struct node { 
        int id;
        int size;
        int index;
        node* next;

        node():id(-1), size(MEMORY_SIZE), index(0), next(nullptr) {};
        node(const node& n):id(n.id), size(n.size), index(n.index), next(nullptr) {};
        node(int i, int j, int k, node* n):id(i), size(j), index(k), next(n) {};
    };

    node* head;
    int size;
    int used;
    const int total;
public:
    LinkedList():size(1), used(0), total(MEMORY_SIZE), head(new node) {};
    LinkedList(const LinkedList& ll):size(ll.size), used(ll.used), total(ll.total) {
        head = new node(*ll.head);
        node* temp = ll.head->next;
        node* temp2 = head;
        while(temp) {
            temp2->next = new node(*temp);
            temp2 = temp2->next;
            temp = temp->next;
        }
    }
    ~LinkedList() {
        clear(head);
        size = 0;
        used = 0;
    }
    
    void clear(node* n) {
        if(n->next) {
            clear(n->next);
            delete n;
        }
    }
    
    void reset() {
        clear(head);
        head = new node();
        size = 1;
        used = 0;
    }

    int allocate(int id, int size) {
        //allocating
        if(size > total - used) {
			//failed allocating
            return -1;
        }
        else {
            this->size++;
            node* temp = head;
            while(temp) {
                if(temp->id == -1 && temp->size >= size) {
                    if(temp->size != size) {
                        node* freeSpace = new node(-1, temp->size - size, temp->index + size, temp->next);
                        temp->next = freeSpace;
                    }
                    temp->id = id;
                    temp->size = size;
                    used += size;
                    break;
                }
                temp = temp->next;
            }//allocated
            if(temp) {
                return temp->index;
            }
            else {
                return -1;
            }
        }
    }

    void deallocate(int id, char* memory) {
        node* temp = head;
        while(temp) {
            if(temp->id == id) {
                temp->id = -1;
                this->size--;
                this->used -= temp->size;
                for(int i = 0; i < temp->size; i++) {
                    memory[temp->index + i] = 'X';
                }
                if(temp->next && temp->next->id == -1) {
                    temp->size += temp->next->size;
                    node* temp2 = temp->next;
                    temp->next = temp2->next;
                    delete temp2;
                }
                if(head != temp) {
                    node* temp2 = head;
                    while(temp2 && temp2->next != temp) {
                        temp2 = temp2->next;
                    }
                    if(temp2 && temp2->id == -1) {
                        temp2->next = temp->next;
                        temp2->size += temp->size;
                        delete temp;
                    }
                }
            }
            temp = temp->next;
        }
    }

    void print() {
        node* temp = head;
        while(temp) {
            cout << "[" << temp->id << "][" << temp->size << "][" << temp->index << "]";
            if(temp->next) {
                cout << "---";
            }
            temp = temp->next;
        }
        cout << endl;
    }
} list;

queue<malloc_req> myqueue; // shared queue
pthread_mutex_t sharedLock = PTHREAD_MUTEX_INITIALIZER; //mutex
pthread_mutex_t screenLock = PTHREAD_MUTEX_INITIALIZER; //mutex
pthread_t server; //server thread handle
sem_t semlist[NUM_THREADS]; //semaphores
sem_t listsem;
bool stop = false;

int thread_message[NUM_THREADS]; //thread memory info
char  memory[MEMORY_SIZE]; //memory size

void release_function(pthread_t* threadArray)
{
    //kill threads and deallocate memory.
    pthread_mutex_lock(&sharedLock);
    stop = true;
    while(myqueue.size() > 0) {
        myqueue.pop();
    }
    
    for(int i = 0; i < NUM_THREADS; i++) {
        pthread_cancel(threadArray[i]/*, NULL*/);
    }
    free(threadArray);
    
    list.reset();
    for(int i = 0; i < MEMORY_SIZE; i++) {
        memory[i] = 'X';
    }
    pthread_mutex_unlock(&sharedLock);
}

void my_malloc(int thread_id, int size)
{
    //add new struct
    pthread_mutex_lock(&sharedLock);
    myqueue.push(malloc_req(thread_id, size));
    pthread_mutex_unlock(&sharedLock);
}

void dump_memory()
{
    //printing memory
    pthread_mutex_lock(&screenLock);
	cout << "List: "<<endl;
    list.print();
    cout << "Memory Dump:" << endl;
    for(int i = 0; i < MEMORY_SIZE; i++) {
        cout << memory[i];
    }
    cout << endl << "*********************************" << endl;
    pthread_mutex_unlock(&screenLock);
}

void free_mem(int _id) {
    sem_wait(&listsem);
    pthread_mutex_lock(&sharedLock);
    list.deallocate(_id, memory);
    pthread_mutex_unlock(&sharedLock);
}

void use_mem(int _id) {
    sleep((rand() % THREAD_SLEEP) + 1);
    free_mem(_id);
    dump_memory();
    sem_post(&listsem);
}

void * server_function(void* id)
{
    while(!stop) {
        if(myqueue.size() > 0) {
            pthread_mutex_lock(&sharedLock);
            malloc_req frontNode = myqueue.front();
            myqueue.pop();
            pthread_mutex_unlock(&sharedLock);
            sem_wait(&listsem);
            pthread_mutex_lock(&sharedLock);
            if((thread_message[frontNode.id] = list.allocate(frontNode.id, frontNode.size)) == -1) {
                //cout << "fail" << endl;
                sem_post(&listsem);
            }
            else {
                //cout << "done" << endl;
            }
            sem_post(&semlist[frontNode.id]);
            pthread_mutex_unlock(&sharedLock);
        }
        else {
            //sleep(0.05);
        }
    }
    return NULL;
}

void * thread_function(void* id)
{
    while(!stop) {
        srand(time(NULL));
        int _id = *(int*)id;
        int size;
        for(int i = 0; i < _id + 1; i++) {
            size = (rand() % (MEMORY_SIZE/3)) + 1;
        }
        my_malloc(_id, size); 
        sem_wait(&semlist[_id]);//block
		//fill the memory with 1s
        int index = thread_message[_id];
        if(index != -1) {
            pthread_mutex_lock(&screenLock);
           //cout << "Thread " << _id << " memory allocated: " << index << " - " << size << endl;
            pthread_mutex_unlock(&screenLock);
            pthread_mutex_lock(&sharedLock);
            for(int i = 0; i < size; i++) {
                memory[index + i] = char('0' + _id);
            }
            dump_memory();
            sem_post(&listsem);
            pthread_mutex_unlock(&sharedLock);
            use_mem(_id);
        }
    }
    return NULL;
}

void init()
{
    pthread_mutex_lock(&sharedLock);//lock
    for(int i = 0; i < NUM_THREADS; i++)//semaphores
    {sem_init(&semlist[i],0,0);}
    sem_init(&listsem, 0, 1);
    for (int i = 0; i < MEMORY_SIZE; i++)//memory
    {char zero = 'X'; memory[i] = zero;}
   	pthread_create(&server,NULL,server_function,NULL); //start the server
    pthread_mutex_unlock(&sharedLock); //then unlock
}

int main (int argc, char *argv[])
{
	//tid's
    pthread_t* threadArray = (pthread_t *)calloc(NUM_THREADS, sizeof(pthread_t));
    //cout << myqueue.size() << endl;
    
    init();	
    
    //create threads w/ tids
    int* index = (int *)calloc(NUM_THREADS, sizeof(int));
    for(int i = 0; i < NUM_THREADS; i++) {
        index[i] = i;
        if(pthread_create(&threadArray[i], NULL, thread_function, &index[i]) != 0) {
            pthread_mutex_lock(&screenLock);
            pthread_mutex_unlock(&screenLock);
        }
    }
    sleep(RUNTIME_DURATION);
    release_function(threadArray);
    dump_memory();
    
    printf("End\n");
}