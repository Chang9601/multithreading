CC=gcc
CFLAGS=-g -Wall -Wshadow -Wvla -Werror -pedantic -Wno-unused-function
DEBUG=gdb
TOOL=valgrind
LDFLAGS=-lpthread

all: prod-cons.exe

prod-cons.exe: prod-cons.o queue.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

prod-cons.o: prod-cons.c
	$(CC) -c $(CFLAGS) $< -o $@

queue.o: queue.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o *.exe