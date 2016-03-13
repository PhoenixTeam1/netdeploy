#include <stdio.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "host.h"

void host_exec(int socket, char* exec_name) {
	int status;
	int pid = fork();
	if (pid < 0) {
		fprintf(stderr, "Fork failed\n");
		return;
	}
	else if (pid > 0) {
		// Wait for program to terminate
		printf("Waiting for client %d to be finished\n", socket);
		waitpid(pid, &status, 0);
		printf("Client %d finished\n", socket);
		//close(socket);
		return;
	}
	// Child branch
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	if (dup(socket) != STDIN_FILENO || dup(socket) != STDOUT_FILENO || dup(socket) != STDERR_FILENO) {
		perror("Error duplicating socket for stdin/out/err");
	}
	printf("Connected to binary: %s\n", exec_name);
	fflush(stdout);
	// Clean this up XXX
	execl(exec_name, exec_name, NULL);
	return;
}

