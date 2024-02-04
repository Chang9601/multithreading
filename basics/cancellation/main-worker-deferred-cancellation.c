#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

#define N_THREADS 5
#define BUF_SIZE 64
#define FILE_SIZE 64

void
mem_cleanup(void *arg) {
  printf("%s...\n", __func__);

  free(arg);

  return;
}

void
file_cleanup(void *arg) {
  printf("%s...\n", __func__);
  
  fclose((FILE *)arg);

  return;
}

void *
write_to_file(void *arg) {
  char file[FILE_SIZE], buf[BUF_SIZE];
  int len, cnt, *tid;
  FILE *fp;

  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

  // 취소를 지연 모드로 설정한다.
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

  cnt = 0;
  tid = (int *)arg;

  pthread_cleanup_push(mem_cleanup, arg);

  sprintf(file, "thread_%d.txt", *tid);

  if ((fp = fopen(file, "w")) == NULL) {
    fprintf(stdout, "오류 발생! 파일 %s 열기 실패, errno = %d\n", file, errno);

    pthread_exit((void *)-1);
  }

  pthread_cleanup_push(file_cleanup, (void *)fp);

  while (1) {
    // 스레드 취소 신호가 전달되었을 때 함수(e.g., sprintf)의 내부 구현을 실행하는 도중에 스레드가 취소될 수 있다.
    // 즉, 스레드가 취소될 경우 정의되지 않은 동작으로 이어질 수 있는 위험한 지점이다.
    // 불변성(invariant) 문제를 해결하기 위해 지연 취소를 사용한다.
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

  pthread_exit(NULL);
}

int
main(int argc, char *argv[]) {
  pthread_t tids[N_THREADS];
  int i, id, ch, err, *tid;

  for (i = 0; i < N_THREADS; i++) {
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