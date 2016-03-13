#ifndef _QUEUE_H 
#define _QUEUE_H 
 
#include <semaphore.h> 
#include <pthread.h> 

typedef struct queue_node_t {
	struct queue_node_t* next;
	struct queue_node_t* prev;
	void* data;
} queue_node_t;

typedef struct queue_t {
	queue_node_t* head;
	queue_node_t* tail;
	sem_t* fill_sem;
	sem_t* limit_sem;
	pthread_mutex_t mutex;
} queue_t;

queue_t* make_queue(const char*, int max_size);
void free_queue(queue_t* queue);
int enqueue(queue_t* queue, void* data);
void* dequeue(queue_t* queue);

#endif
