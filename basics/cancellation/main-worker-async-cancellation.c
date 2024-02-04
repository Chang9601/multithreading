#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#define N_THREADS 5
#define BUF_SIZE 64
#define FILE_SIZE 64

void *
write_to_file(void *arg) {
  char file[FILE_SIZE], buf[BUF_SIZE];
  int len, cnt, *tid;
  FILE *fp;

  // 스레드를 취소 가능한 스레드로 표시한다.
  // PTHREAD_CANCEL_ENABLE: 취소 가능한 스레드
  // PTHREAD_CANCEL_DISABLE: 취소할 수 없는 스레드
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

  // 취소 가능한 스레드의 취소 모드를 설정한다.
  // PTHREAD_CANCEL_ASYNCHRONOUS: 비동기 취소
  // PTHREAD_CANCEL_DEFERRED: 지연 취소
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

  cnt = 0;
  tid = (int *)arg;

  sprintf(file, "thread_%d.txt", *tid);

  if ((fp = fopen(file, "w")) == NULL) {
    fprintf(stdout, "오류 발생! 파일 %s 열기 실패, errno = %d\n", file, errno);
    
    return (void *)-1;
  }

  while (1) {
    len = sprintf(buf, "%d. 스레드 %d\n", cnt++, *tid);
    fwrite(buf, sizeof(char), len, fp);
    fflush(fp);
    sleep(1);
  }

  return NULL;
}

int
main(int argc, char *argv[]) {
  pthread_t tids[N_THREADS];
  int i, id, ch, err, *tid;

  // N_THREADS 개수의 스레드를 생성한다.
  // 스레드 아이디를 인자로 전달한다.
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
        // 스레드가 취소 가능하지(cancellable) 않으면 작동하지 않는다.
        pthread_cancel(tids[id]);
        break;
      default:
        continue;
    }
  }

  exit(EXIT_SUCCESS);
}