CC=gcc
CFLAGS=-g -Wall -Wshadow -Wvla -Werror -pedantic -Wno-unused-function
DEBUG=gdb
TOOL=valgrind
LDFLAGS=-lpthread
SRC_C=dp.c
OBJ_O=dp.o
EXECUTABLE=dp.exe

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJ_O)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

$(OBJ_O): $(SRC_C)
	$(CC) -c $(CFLAGS) $< -o $@

debug:
	$(DEBUG) $(EXECUTABLE)

clean:
	rm -f $(OBJ_O) $(EXECUTABLE)

run: $(EXECUTABLE)
	$(TOOL) ./$(EXECUTABLE)
