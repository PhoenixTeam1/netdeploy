#ifndef _SERVER_H
#define _SERVER_H

int server_create(int port);
void server_run(int server_socket, int threads, int max_queue_size, char* exec_name);

#endif
