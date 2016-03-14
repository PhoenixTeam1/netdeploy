#include <stdio.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/tcp.h>
#include <signal.h>
#include "host.h"

#define TEMP_BUF_LEN	1024

static int recv_comm(int socket, unsigned char* buffer, int length);
static int recv_comm_until(int socket, unsigned char** buffer, char* needle);
static int send_comm(int socket, unsigned char* buffer, int length);

int host_exec(int socket, char* exec_name) {
	int status;
	int ret;
	int flag;
	char restart_prompt[] = "[netdeploy] Restart process? (y/n): ";
	char exit_message[1024];
	char* answer_buf;
	char* env[] = {"LD_PRELOAD=./custom_links.so",NULL};
	int pid = fork();
	flag = 1;
	if (pid < 0) {
		fprintf(stderr, "Fork failed\n");
		return 0;
	}
	else if (pid > 0) {
		// Wait for program to terminate
		printf("Waiting for client %d to be finished\n", socket);
		waitpid(pid, &status, 0);
		if (WIFSIGNALED(status)) {
			ret = WTERMSIG(status);
			snprintf(exit_message, sizeof(exit_message),
			"\n[netdeploy] Hosted process was killed by signal: %d (%s)\n",
			ret, strsignal(ret));
			printf("%s", exit_message);
			send_comm(socket, (unsigned char*)exit_message, strlen(exit_message));
		}
		if (WIFEXITED(status)) {
			ret = WEXITSTATUS(status);
			snprintf(exit_message, sizeof(exit_message),
			"\n[netdeploy] Hosted process exited with status code: %d (%s)\n",
			ret, ret == EXIT_SUCCESS ? "EXIT_SUCCESS" : "EXIT_FAILURE");
			printf("%s", exit_message);
			send_comm(socket, (unsigned char*)exit_message, strlen(exit_message));
		}
		send_comm(socket, (unsigned char*)restart_prompt, strlen(restart_prompt));
		recv_comm_until(socket, (unsigned char**)&answer_buf, "\n");
		if (answer_buf[0] == 'y' || answer_buf[0] == 'Y') {
			free(answer_buf);
			printf("Client %d is restarting hosted process\n", socket);
			return 1; // Return with 1 to handle client again
		}
		free(answer_buf);
		printf("Client %d finished\n", socket);
		return 0; // Done with this client
	}
	// Child branch
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(int));
	if (dup(socket) != STDIN_FILENO || 
		dup(socket) != STDOUT_FILENO ||
		dup(socket) != STDERR_FILENO) {
		perror("Error duplicating socket for stdin/out/err");
	}
	printf("[netdeploy] Connected to binary \"%s\"\n", exec_name);
	fflush(stdout);
	// XXX add proper env and args (requires taking more args from main.c)
	// add failure checks
	execle(exec_name, exec_name, NULL, env);
	return 0;
}

int recv_comm_until(int socket, unsigned char** buffer, char* needle) {
	int total_bytes_read;
	int bytes_read;
	int buffer_length = TEMP_BUF_LEN;
	char temp_buffer[TEMP_BUF_LEN];
	total_bytes_read = 0;
	*buffer = (unsigned char*)calloc(sizeof(unsigned char),buffer_length);
	while (strstr((char*)*buffer, needle) == NULL) {
		bytes_read = recv(socket, temp_buffer, TEMP_BUF_LEN, 0);
		if (bytes_read < 0) {
			if (errno == EINTR) {
				continue; // continue upon interrupt
			}
			else { // something else happened, abort
				return -2;
			}
		}
		else if (bytes_read == 0) {
			return -1;
		}

		if ((total_bytes_read + bytes_read) > buffer_length) {
			*buffer = (unsigned char*)realloc(*buffer, (total_bytes_read + bytes_read) * 2);
		}
		memcpy(&(*buffer)[total_bytes_read], temp_buffer, bytes_read);
		total_bytes_read += bytes_read;
	}
	return 0;
}

int recv_comm(int socket, unsigned char* buffer, int length) {
	unsigned char* ptr;
	int bytes_left;
	int bytes_read;
	ptr = buffer;
	bytes_left = length;
	while (bytes_left) {
		bytes_read = recv(socket, ptr, bytes_left, 0);
		if (bytes_read < 0) {
			if (errno == EINTR) {
				continue; // continue upon interrupt
			}
			else { // something else happened, abort
				return -2;
			}
		}
		else if (bytes_read == 0) {
			return -1;
		}
		ptr += bytes_read;
		bytes_left -= bytes_read;
	}
	return 0;
}

int send_comm(int socket, unsigned char* buffer, int length) {
	unsigned char* ptr;
	int bytes_left;
	int bytes_sent;
	ptr = buffer;
	bytes_left = length;
	while (bytes_left) {
		bytes_sent = send(socket, ptr, bytes_left, 0);
		if (bytes_sent < 0) {
			if (errno == EINTR) {
				continue; // continue upon interrupt
			}
			else {
				perror("write");
				return -2; // something else happened, abort
			}
		}
		else if (bytes_sent == 0) {
			return -1;
		}
		ptr += bytes_sent;
		bytes_left -= bytes_sent;
	}
	return 0;
}

