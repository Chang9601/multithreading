#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#define N_WORKERS 5
#define BUF_SIZE 64
#define FILE_SIZE 64

pthread_t workers[N_WORKERS];

void *
write_to_file(void *arg) {
  char file[FILE_SIZE];
  char buf[BUF_SIZE];
  int len;
  int cnt = 0;

  // 스레드를 취소 가능한 스레드로 표시한다.
  // PTHREAD_CANCEL_ENABLE: 취소 가능한 스레드
  // PTHREAD_CANCEL_DISABLE: 취소할 수 없는 스레드
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

  // 취소 모드를 설정한다.
  // PTHREAD_CANCEL_ASYNCHRONOUS: 비동기 취소
  // PTHREAD_CANCEL_DEFERRED: 지연 취소
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

  int *tid = (int *)arg;

  sprintf(file, "thread_%d.txt", *tid);
  FILE *fp;

  if ((fp = fopen(file, "w")) == NULL) {
    fprintf(stdout, "오류 발생! 파일 %s 열기 실패, errno = %d\n", file, errno);
    exit(EXIT_FAILURE);
  }

  // 각 스레드는 버퍼의 내용을 해당하는 파일에 작성한다.
  while (1) {
    len = sprintf(buf, "%d. 스레드 %d\n", cnt++, *tid);
    fwrite(buf, sizeof(char), len, fp);
    fflush(fp);
    sleep(1);
  }

  exit(EXIT_SUCCESS);
}

int
main(int argc, char *argv[]) {

  int i;
  int *tid = 0;

  // N_WORKERS 개수의 스레드를 생성한다.
  // 스레드 아이디를 인자로 전달한다.
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
      printf("오류 발생! 유효하지 않은 스레드\n");
      exit(EXIT_FAILURE);    
    }
    printf("\n");

    switch (ch)
    {
      case 1:
        // 스레드가 취소 가능하지 않으면 작동하지 않는다.
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