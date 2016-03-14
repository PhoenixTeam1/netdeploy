#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <argp.h>
#include "server.h"

#define DEFAULT_THREAD_COUNT	10
#define DEFAULT_PORT		8888

static error_t parse_opt(int key, char* arg, struct argp_state* state);
static void print_exec_args(char** argv);

const char* argp_program_version = "0.1";
const char* argp_program_bug_address = "mark@markoneill.name";
static char doc[] = "\n\
Netdeploy by Mark O\'Neill\n\n\
A tool that enables local applications that use stdin/out to be deployed within \
a network context, allowing users to access them remotely. Operation is similar \
to \"netcat -e\" but provides support for multiple simultaneous instances of the \
target program to be accessed by multiple clients, via threading.";
static char args_doc[] = "FILENAME";

static struct argp_option options[] = {
	{"threads",	't',	"COUNT",	0, \
"(optional) Size of thread pool, which correlates to number of simultaneous users (default 10)"},
	{"port",	'p',	"PORT",		0, \
"(optional) Port to listen on (default 8888)"},
	{ 0 }
};

typedef struct arguments_t {
	char* exec_name;
	char** exec_args;
	unsigned int num_threads;
	int port;
} arguments_t;

static struct argp argp = {options, parse_opt, args_doc, doc};

int main(int argc, char* argv[]) {
	int server_socket;
	arguments_t args;

	// Defaults
	args.exec_name = NULL;
	args.num_threads = DEFAULT_THREAD_COUNT;
	args.port = DEFAULT_PORT;

	argp_parse(&argp, argc, argv, 0, 0, &args);
	printf("Deploying executable %s on port %d with thread pool size of %d\n", 
		args.exec_name, args.port, args.num_threads);
	print_exec_args(args.exec_args);
	server_socket = server_create(args.port);
	server_run(server_socket, args.num_threads, args.num_threads, args.exec_name, args.exec_args);
	return EXIT_SUCCESS;
}

error_t parse_opt(int key, char* arg, struct argp_state* state) {
	char* slash;
	struct arguments_t* arguments = state->input;
	switch(key) {
		case 't':
			arguments->num_threads = arg ? atoi(arg) : DEFAULT_THREAD_COUNT;
			break;
		case 'p':
			arguments->port = arg ? atoi(arg) : DEFAULT_PORT;
			break;
		case ARGP_KEY_ARG: /* Normal arguments here */
			if (state->arg_num == 0) {
				arguments->exec_name = arg;
				arguments->exec_args = &state->argv[(state->next)-1];
				// remove path from first arg
				slash = strrchr(arguments->exec_args[0], '/');
				if (slash) arguments->exec_args[0] = slash+1;
				state->next = state->argc; // end parsing of arguments
				// (the rest are all for the hosted process)
			}
			break;
		case ARGP_KEY_END:
			if (state->arg_num < 1) {
				argp_usage(state);
			}
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

void print_exec_args(char** argv) {
	char* cur_arg;
	int i;
	i = 0;
	printf("Hosted executable arguments are");
	while ((cur_arg = argv[i++]) != NULL) {
		printf(" %s", cur_arg);
	}
	printf("\n");
	return;
}

