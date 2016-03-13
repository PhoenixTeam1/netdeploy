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
	va_list list;
	char* parg;
	int ret;
	typeof(printf) *orig_printf;
	va_start(list, format);
	vasprintf(&parg, format, list);
	va_end(list);
	orig_printf = dlsym(RTLD_NEXT, "printf");
	//(*orig_printf)("Overriding printf\n");
	ret = (*orig_printf)("%s", parg);
	free(parg);
	fflush(0); // Force application to flush buffer after every printf
	return ret; 
}
