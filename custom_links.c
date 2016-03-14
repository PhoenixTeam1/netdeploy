#define _GNU_SOURCE

#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <stdarg.h>

int setvbuf(FILE *stream, char *buf, int mode, size_t size) {
	int (*orig_setvbuf)(FILE*, char*, int, size_t);
	//printf("Custom setvbuf triggered\n");
	orig_setvbuf = dlsym(RTLD_NEXT, "setvbuf");
	return (*orig_setvbuf)(stream, buf, _IONBF, size);
}

int printf(const char* format, ...) {
	va_list args;
	int ret;
	typeof(printf) *orig_printf;
	va_start(args, format);
	ret = vprintf(format, args);
	va_end(args);
	orig_printf = dlsym(RTLD_NEXT, "printf");
	//(*orig_printf)("Overriding printf\n");
	fflush(0); // Force application to flush buffer after every printf
	return ret; 
}

// XXX add more overrides as necessary (fprintf for example)
