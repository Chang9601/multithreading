#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#define N_WORKERS 5
#define BUF_LEN 64
#define FILE_LEN 64

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
  char file_name[FILE_LEN];
  char buf[BUF_LEN];
  int len;
  int cnt = 0;

  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

  // 취소를 지연 모드로 설정한다.
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

  int *tid = (int *)arg;

  pthread_cleanup_push(mem_cleanup, arg);

  sprintf(file_name, "thread_%d.txt", *tid);
  FILE *fp;

  if ((fp = fopen(file_name, "w")) == NULL) {
    fprintf(stdout, "오류 발생! 파일 %s 열기 실패, errno = %d\n", file_name, errno);

    pthread_exit((void *)0);
  }

  pthread_cleanup_push(file_cleanup, fp);

  // 스레드 취소 신호가 전달되었을 때 함수(e.g., sprintf)의 내부 구현을 실행하는 도중에 스레드가 취소될 수 있다.
  // 즉, 스레드가 취소될 경우 정의되지 않은 동작으로 이어질 수 있는 위험한 지점이다.
  // 불변성 문제를 해결하기 위해 지연 취소를 사용한다.
  while (1) {
    len = sprintf(buf, "%d. 스레드 %d\n", cnt++, *tid);
    fwrite(buf, sizeof(char), len, fp);
    fflush(fp);
    sleep(1);
    // 스레드가 pthread_testcancel() 함수를 실행할 때마다 보류 중인 취소 신호가 있는지 확인한다.
    // 보류 중인 취소 신호가 있다면, 스레드는 그 줄에서 바로 취소된다.
    // 함수 호출에서 불변성 문제가 발생하지 않는다.
    // 함수가 완전히 실행되고 실행이 구현 내의 무작위 지점에서 취소되지 않기 때문이다.
    pthread_testcancel();
  }

  pthread_cleanup_pop(1);
  pthread_cleanup_pop(1);

  exit(EXIT_SUCCESS);
}

int
main(int argc, char *argv) {

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