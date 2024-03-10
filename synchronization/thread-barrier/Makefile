CC=gcc
CFLAGS=-g -Wall -Wshadow -Wvla -Werror -pedantic -Wno-unused-function
DEBUG=gdb
TOOL=valgrind
LDFLAGS=-lpthread
EXECUTABLE=main.exe

all: $(EXECUTABLE)

$(EXECUTABLE): main.o thr-barrier.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

main.o: main.c
	$(CC) -c $(CFLAGS) $< -o $@

thr-barrier.o: thr-barrier.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o *.exe

run: $(EXECUTABLE)
	$(TOOL) ./$(EXECUTABLE)