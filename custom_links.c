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
	//typeof(printf) *orig_printf;
	va_start(args, format);
	ret = vprintf(format, args);
	va_end(args);
	//orig_printf = dlsym(rtld_next, "printf");
	//(*orig_printf)("overriding printf\n");
	fflush(0); // force application to flush buffer after every printf
	return ret; 
}

int fprintf(FILE* stream, const char* format, ...) {
	va_list args;
	int ret;
	//typeof(fprintf) *orig_fprintf;
	va_start(args, format);
	ret = vfprintf(stream, format, args);
	va_end(args);
	//orig_fprintf = dlsym(rtld_next, "fprintf");
	//(*orig_printf)("overriding printf\n");
	if (stream == stdout || stream == stderr) {
		fflush(0); // force application to flush buffer after every printf
	}
	return ret; 
}

int vprintf(const char* format, va_list arg) {
	int ret;
	typeof(vprintf) *orig_vprintf;
	orig_vprintf = dlsym(RTLD_NEXT, "vprintf");
	ret = (*orig_vprintf)(format, arg);
	fflush(0);
	return ret;
}

int vfprintf(FILE* stream, const char* format, va_list arg) {
	int ret;
	typeof(vfprintf) *orig_vfprintf;
	orig_vfprintf = dlsym(RTLD_NEXT, "vfprintf");
	ret = (*orig_vfprintf)(stream, format, arg);
	fflush(0);
	return ret;
}

// XXX add more overrides as necessary (cout?)
