#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "server.h"
#include "queue.h"
#include "host.h"

typedef struct thread_param_t {
	queue_t* queue;
	char* exec_name;
} thread_param_t;

static void* thread_loop(void* data);

int server_create(int port) {
	int server_socket;
	struct sockaddr_in server_addr;
	int reuse = 1;

	// setup socket address structure
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	// create socket
	server_socket = socket(PF_INET, SOCK_STREAM, 0);
	if (!server_socket) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	// set socket to immediately reuse port when the application closes
	if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	// call bind to associate the socket with our local address and port
	if (bind(server_socket, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		perror("bind");
		exit(EXIT_FAILURE);
	}

	// convert the socket to listen for incoming connections
	if (listen(server_socket, SOMAXCONN) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}
	printf("Server socket bound and listening\n");
	return server_socket;
}

void server_run(int server_socket, int threads, int max_queue_size, char* exec_name) {
	int i;
	queue_t* queue;
	int client_socket;
	struct sockaddr_in client_addr;
	socklen_t client_len;
	pthread_t* thread_pool;
	thread_param_t* params;

	client_len = sizeof(client_addr);
	
	// Set up Threading
	queue = make_queue("netdeploy", max_queue_size);
	thread_pool = (pthread_t*)malloc(sizeof(pthread_t) * threads);
	params = (thread_param_t*)malloc(sizeof(thread_param_t) * threads);
	for (i = 0; i < threads; i++) {
		params[i].queue = queue;
		params[i].exec_name = exec_name;
		pthread_create(&thread_pool[i], NULL, thread_loop, (void*)&params[i]);
	}

	// Continually enqueue
	printf("Running worker threads and accepting clients\n");
	while (1) {
		client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
		if (client_socket > 0) {
			enqueue(queue, (void*)client_socket);
		}
		else {
			break;
		}
	}

	for (i = 0; i < threads; i++) {
		pthread_join(thread_pool[i], NULL);
	}
	free_queue(queue);
	free(params);
	free(thread_pool);
	return;
}

void* thread_loop(void* data) {
	int client;
	char* exec_name;
	queue_t* queue;
	thread_param_t* params = (thread_param_t*)data;
	queue = params->queue;
	exec_name = params->exec_name;
	while (1) {
		client = (int)dequeue(queue);
		while (host_exec(client, exec_name) == 1) {};
		close(client);
	}
	return NULL;

}

