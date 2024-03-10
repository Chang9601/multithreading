CC=gcc
CFLAGS=-g -Wall -Wshadow -Wvla -Werror -pedantic -Wno-unused-function
DEBUG=gdb
TOOL=valgrind
LDFLAGS=-lpthread
SRC_C=thread.c
OBJ_O=thread.o
EXECUTABLE=thread.exe

# 모든 대상(target)을 실행한다.
# 1번째로 나열된 규칙이기 때문에 대상을 명시하지 않고 make를 호출하면 기본적으로 실행된다.
all: $(EXECUTABLE)

# $@는 규칙의 대상 파일 이름.
$(EXECUTABLE): $(OBJ_O)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

# $<는 첫 번째 필수조건(prerequisite)의 이름.
$(OBJ_O): $(SRC_C)
	$(CC) -c $(CFLAGS) $< -o $@

debug:
	$(DEBUG) $(EXECUTABLE)

clean:
	rm -f $(OBJ_O) $(EXECUTABLE)

run: $(EXECUTABLE)
	$(TOOL) ./$(EXECUTABLE)
