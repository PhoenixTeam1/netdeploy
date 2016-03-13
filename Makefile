CC = gcc
CCFLAGS = -Wall -O3 -fpic
LIBS = -lpthread

NETDEPLOY_SRC = main.c \
		server.c \
		queue.c \
		host.c

NETDEPLOY_OBJ = $(NETDEPLOY_SRC:%.c=%.o)
NETDEPLOY_EXE = netdeploy

ECHO_TEST_SRC = echo_test.c
ECHO_TEST_OBJ = $(ECHO_TEST_SRC:%.c=%.o)
ECHO_TEST_EXE = echo_test

CUSTOM_LINKS_SRC = custom_links.c
CUSTOM_LINKS_OBJ = $(CUSTOM_LINKS_SRC:%.c=%.o)
CUSTOM_LINKS_SO = custom_links.so

all: $(NETDEPLOY_EXE) $(ECHO_TEST_EXE) $(CUSTOM_LINKS_SO)

$(NETDEPLOY_EXE) : $(NETDEPLOY_OBJ)
	$(CC) $(CCFLAGS) $^ -o $@ $(LIBS)

$(ECHO_TEST_EXE) : $(ECHO_TEST_OBJ)
	$(CC) $(CCFLAGS) $^ -o $@ $(LIBS)

$(CUSTOM_LINKS_SO) : $(CUSTOM_LINKS_OBJ)
	$(CC) $(CCFLAGS) -shared $^ -o $@ -ldl

%.o : %.c
	$(CC) $(CCFLAGS) -c $< -o $@

clean:
	rm -rf *.o $(NETDEPLOY_EXE) $(ECHO_TEST_EXE) $(CUSTOM_LINKS_SO)
