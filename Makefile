CC = gcc
CCLFAGS = -Wall -O3
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

all: $(NETDEPLOY_EXE) $(ECHO_TEST_EXE)

$(NETDEPLOY_EXE) : $(NETDEPLOY_OBJ)
	$(CC) $(CCFLAGS) $^ -o $@ $(LIBS)


$(ECHO_TEST_EXE) : $(ECHO_TEST_OBJ)
	$(CC) $(CCFLAGS) $^ -o $@ $(LIBS)

%.o : %.c
	$(CC) $(CCFLAGS) -c $< -o $@

clean:
	rm -rf *.o $(NETDEPLOY_EXE) $(ECHO_TEST_EXE)
