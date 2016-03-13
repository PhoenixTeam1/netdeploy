#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h> /* O_CREAT constant */
#include <sys/stat.h> /* S_IRWXU constant */
#include <semaphore.h>
#include "queue.h"

static queue_node_t* make_queue_node(void* data);

/**
 * Create a new empty queue
 * @param max_size maximum size of queue (0 for unlimited)
 * @returns queue pointer or NULL on failure
 */
queue_t* make_queue(const char* name, int max_size) {
	char sem_fill_name[32];
	char sem_limit_name[32];
	queue_t* queue;
	sem_t* fill_sem;
	sem_t* limit_sem;
	sprintf(sem_fill_name, "%s_fill_sem", name);
	fill_sem = sem_open(sem_fill_name, O_CREAT, S_IRWXU, 0);
	if (fill_sem == SEM_FAILED) {
		fprintf(stderr, "Failed to create queue semaphore %s: %s\n", sem_fill_name, strerror(errno));
		return NULL;
	}
	limit_sem = NULL;
	if (max_size != 0) {
		sprintf(sem_limit_name, "%s_limit_sem", name);
		limit_sem = sem_open(sem_limit_name, O_CREAT, S_IRWXU, max_size);
		if (limit_sem == SEM_FAILED) {
			fprintf(stderr, "Failed to create queue semaphore %s: %s\n", sem_limit_name, strerror(errno));
			return NULL;
		}
	}
	queue = (queue_t*)malloc(sizeof(queue_t));
	if (queue == NULL) {
		fprintf(stderr, "Failed to allocate space for queue %s\n", name);
		return NULL;
	}
	if (pthread_mutex_init(&queue->mutex, NULL) != 0) {
		fprintf(stderr, "Failed to create mutex for queue %s\n", name);
		free(queue); /* free allocated memory since this happened after malloc */
		return NULL;
	}
	queue->head = NULL;
	queue->tail = NULL;
	queue->fill_sem = fill_sem;
	queue->limit_sem = limit_sem;
	return queue;
}

/**
 * Empties queue and frees all nodes
 * and queue itself
 */
void free_queue(queue_t* queue) {
	queue_node_t* current;
	queue_node_t* next;
	if (queue == NULL) {
		return;
	}
	current = queue->head;
	while (current != NULL) {
		next = current->next;
		/* We're not freeing the contents of nodes on purpose.
		   It was created by others and is their responsibility. */
		free(current);
		current = next;
	}
	if (sem_close(queue->fill_sem) == -1) {
		fprintf(stderr, "Failed to close fill semaphore: %s\n", strerror(errno));
	}
	if (queue->limit_sem != NULL) {
		if (sem_close(queue->limit_sem) == -1) {
			fprintf(stderr, "Failed to close limit semaphore: %s\n", strerror(errno));
		}
	}
	if (pthread_mutex_destroy(&queue->mutex) != 0) {
		fprintf(stderr, "Failed to destroy queue mutex\n");
	}
	free(queue);
	return;
}

/**
 * Creates a queue node
 * @returns queue_node_t pointer or NULL on failure
 */
queue_node_t* make_queue_node(void* data) {
	queue_node_t* node;
	node = (queue_node_t*)malloc(sizeof(queue_node_t));
	if (node == NULL) {
		return NULL;
	}
	node->next = NULL;
	node->prev = NULL;
	node->data = data;
	return node;
}

/**
 * Adds an item to the specified queue
 * @returns 1 on success, 0 on failure
 */
int enqueue(queue_t* queue, void* data) {
	queue_node_t* new_node;
	pthread_mutex_lock(&queue->mutex);
	new_node = make_queue_node(data);
	if (new_node == NULL) {
		pthread_mutex_unlock(&queue->mutex);
		return 0;
	}

	sem_wait(queue->limit_sem);
	// If queue is empty
	if (queue->head == NULL) {
		assert(queue->tail == NULL);
		queue->head = new_node;
		queue->tail = new_node;
		pthread_mutex_unlock(&queue->mutex);
		sem_post(queue->fill_sem);
		return 1;
	}

	// If queue already has something in it
	assert(queue->tail->next == NULL);
	new_node->prev = queue->tail;
	queue->tail->next = new_node;
	queue->tail = new_node;
	pthread_mutex_unlock(&queue->mutex);
	sem_post(queue->fill_sem);
	return 1;
}

/**
 * Returns the first element on the queue and
 * removes it
 * @returns pointer to first item or NULL on failure
 */
void* dequeue(queue_t* queue) {
	queue_node_t* node;
	void* data;
	sem_wait(queue->fill_sem);
	pthread_mutex_lock(&queue->mutex);
	if (queue->head == NULL) {
		pthread_mutex_unlock(&queue->mutex);
		return NULL;
	}
	// Get data
	node = queue->head;
	data = node->data;

	// If this was the last item in the queue
	if (queue->tail == node) {
		queue->tail = NULL;
	}

	// Make next element new head
	queue->head = node->next;
	if (queue->head != NULL) {
		queue->head->prev = NULL;
	}
	free(node);
	pthread_mutex_unlock(&queue->mutex);
	sem_post(queue->limit_sem);
	return data;
}

