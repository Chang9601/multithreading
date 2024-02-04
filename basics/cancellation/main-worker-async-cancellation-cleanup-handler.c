#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#define N_THREADS 5
#define BUF_SIZE 64
#define FILE_SIZE 64

void
mem_cleanup(void *arg) {
  printf("%s...\n", __func__);

  free(arg);
}

void
file_cleanup(void *arg) {
  printf("%s...\n", __func__);
  
  fclose((FILE *)arg);
}

void *
write_to_file(void *arg) {
  char file[FILE_SIZE], buf[BUF_SIZE];
  int len, cnt, *tid;
  FILE *fp;

  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

  cnt = 0;
  // 힙 메모리에 존재하는 arg, 스레드가 취소 신호를 받으면 메모리 해제가 필요하다.
  tid = (int *)arg;

  // 스레드가 취소 신호를 받으면 정리 함수는 포인터 주소의 메모리를 해제한다.
  pthread_cleanup_push(mem_cleanup, arg);

  sprintf(file, "thread_%d.txt", *tid);

  if ((fp = fopen(file, "w")) == NULL) {
    fprintf(stdout, "오류 발생! 파일 %s 열기 실패, errno = %d\n", file, errno);
    // 만약 파일을 열 수 없으면 arg 변수를 해제해야 하는데 스레드가 반환문(return (void *)1;)을 통해 종료될 때 스레드 정리 스택에 있는 정리 함수는 호출되지 않는다.
    // 하지만, pthread_exit()를 사용하여 스레드를 종료하면 정리 함수가 호출된다.
    pthread_exit((void *)-1);
  }

  // 열린 파일은 스레드가 취소될 때 닫혀져야 한다.
  // 즉, 파일이 성공적으로 열리면 스레드는 파일을 닫는 정리 함수가 필요하다.
  pthread_cleanup_push(file_cleanup, fp);
  
  // 스레드가 취소 신호를 받았을 때 while 문 안의 코드를 실행 중이다.
  while (1) {
    len = sprintf(buf, "%d. 스레드 %d\n", cnt++, *tid);
    fwrite(buf, sizeof(char), len, fp);
    fflush(fp);
    sleep(1);
  }

  // 정리 함수를 스택에 넣었기에 스레드가 성공적으로 종료되면 정리 함수를 스레드 정리 함수 스택에서 제거하기 위해 호출해야 한다.
  // 두 번 호출하는 이유는 두 개의 정리 함수를 스레드 정리 함수 스택에 넣었기 때문이다.
  // 인자로 0을 전달하면 스레드 정리 함수 스택의 맨 위에서 정리 함수를 제거한다.
  // 인자로 0 이외의 값을 전달하면 스레드 정리 함수 스택의 맨 위에서 정리 함수를 제거하고 호출한다.
  pthread_cleanup_pop(1);
  pthread_cleanup_pop(1);

  pthread_exit(NULL);
}

int
main(int argc, char *argv[]) {
  pthread_t tids[N_THREADS];
  int i, id, ch, err, *tid;

  for (i = 0; i< N_THREADS; i++) {
    tid = calloc(1, sizeof(*tid));
    *tid = i;

    err = pthread_create(&tids[i], NULL, write_to_file, (void *)tid);
    if (err != 0) {
      printf("오류 발생! 스레드 생성 실패, errno = %d\n", err);
      
      exit(EXIT_FAILURE);      
    }
  }

  while (1) {
    printf("1. 스레드를 취소한다.\n");
    scanf("%d", &ch);

    printf("스레드 번호를 입력하시오. [0-%d]: ", N_THREADS - 1);
    scanf("%d", &id);

    if (id < 0 || id >= N_THREADS) {
      printf("유효하지 않은 스레드 번호!\n");
      continue;
    }

    switch (ch) {
      case 1:
        pthread_cancel(tids[id]);
        break;
      default:
        continue;
    }
  }

  exit(EXIT_SUCCESS);
}