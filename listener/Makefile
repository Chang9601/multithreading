CC=gcc
CFLAGS=-g -Wall -Wshadow -Wvla -Werror -pedantic
LDFLAGS=-lpthread

all: listener.exe sender.exe

listener.exe: listener.o udp.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

sender.exe: sender.o udp.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

listener.o: listener.c
	$(CC) $(CFLAGS) -c $< -o $@

sender.o: sender.c
	$(CC) $(CFLAGS) -c $< -o $@

udp.o: udp.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.exe
