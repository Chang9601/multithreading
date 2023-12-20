#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#define N_WORKERS 5
#define BUF_SIZE 64
#define FILE_SIZE 64

pthread_t workers[N_WORKERS];

void
mem_cleanup(void *arg) {
  printf("%s...\n", __FUNCTION__);
  free(arg);
}

void
file_cleanup(void *arg) {
  printf("%s...\n", __FUNCTION__);
  fclose((FILE *)arg);
}

void *
write_to_file(void *arg) {
  char file[FILE_SIZE];
  char buf[BUF_SIZE];
  int len;
  int cnt = 0;

  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

  // 힙 메모리에 존재하는 arg, 스레드가 취소 신호를 받으면 메모리 해제가 필요하다.
  int *tid = (int *)arg;

  // 스레드가 취소 신호를 받으면 정리 함수는 포인터 주소의 메모리를 해제한다.
  pthread_cleanup_push(mem_cleanup, arg);

  sprintf(file, "thread_%d.txt", *tid);
  FILE *fp;

  if ((fp = fopen(file, "w")) == NULL) {
    fprintf(stdout, "오류 발생! 파일 %s 열기 실패, errno = %d\n", file, errno);
    // 만약 파일을 열 수 없으면 arg를 해제해야 하는데 스레드가 반환문을 통해 종료될 때 스레드 정리 스택에 있는 정리 함수들 호출되지 않는다.
    // exit(EXIT_FAILURE);
    // 하지만, pthread_exit()를 사용하여 스레드를 종료하면 정리 함수가 호출된다.
    pthread_exit((void *)0);
  }

  // 열린 파일은 스레드가 취소될 때 닫혀져야 한다.
  // 즉, 파일이 성공적으로 열리면 스레드는 파일을 닫는 정리 함수가 필요하다.
  pthread_cleanup_push(file_cleanup, fp);
  
  while (1) {
    len = sprintf(buf, "%d. 스레드 %d\n", cnt++, *tid);
    fwrite(buf, sizeof(char), len, fp);
    fflush(fp);
    sleep(1);
  }

  // 정리 함수를 스택에 넣었기에 스레드가 성공적으로 종료되면 정리 함수를 스레드 정리 함수 스택에서 제거하기 위해 호출해야 한다.
  // 두 번 호출하는 이유는 두 개의 정리 함수를 스레드 정리 함수 스택에 넣었기 때문이다.
  // 인자로 0을 전달하면 스레드 정리 함수 스택의 맨 위에서 함수를 제거한다.
  // 인자로 0 이외의 값이 전달되면 스레드 정리 함수 스택의 맨 위에서 정리 함수를 빼내는 것뿐만 아니라 해당 함수를 호출한다.
  pthread_cleanup_pop(1);
  pthread_cleanup_pop(1);

  exit(EXIT_SUCCESS);
}

int
main(int argc, char *argv[]) {

  int i;
  int *tid = 0;

  for (i = 0; i< N_WORKERS; i++) {
    tid = calloc(1, sizeof(*tid));
    *tid = i;
    pthread_create(&workers[i], NULL, write_to_file, (void *)tid);
  }

  int ch;
  int id;

  while (1) {
    printf("1. 스레드를 취소한다.\n");
    scanf("%d", &ch);
    printf("스레드 번호를 입력하시오. [0-%d]: ", N_WORKERS - 1);
    scanf("%d", &id);

    if (id < 0 || id >= N_WORKERS) {
      printf("오류 발생! 유효하지 않은 스레드, 다시 입력하시오!\n");
      exit(EXIT_FAILURE);    
    }
    printf("\n");

    switch (ch)
    {
      case 1:
        pthread_cancel(workers[id]);
        break;
      default:
        continue;
    }
  }

  for (i = 0; i< N_WORKERS; i++) {
    pthread_join(workers[i], NULL);
  }

  exit(EXIT_SUCCESS);
}