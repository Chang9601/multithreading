#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int N;

void *
thr_fn(void *arg) {
  pthread_t next_tid;
  int cnt;

  cnt = *(int *)arg;
  free(arg);

  if (cnt < N) {
    int err, *next_cnt;
    
    next_cnt = calloc(1, sizeof(*next_cnt));
    *next_cnt = cnt + 1;

    // 1번 스레드는 2번 스레드를, 2번 스레드는 3번 스레드를, N-1번 스레드는 N번 스레드를 생성한다.
    err = pthread_create(&next_tid, NULL, thr_fn, (void *)next_cnt);
    if (err != 0) {
      printf("오류 발생! 스레드 생성 실패, errno = %d\n", err);

      return (void *)-1;
    }

    // 2번 스레드는 1번 스레드에, 3번 스레드는 2번 스레드에, ... N번 스레드는 N-1번 스레드에 결합한다.
    err = pthread_join(next_tid, NULL);
    if (err != 0) {
      printf("오류 발생! 스레드 결합 실패, errno = %d\n", err);

      return (void *)-2;
    }
  }

  printf("%d\n", cnt);

  return NULL;
}

void
create_thr(pthread_t *tid, int cnt) {
  int err, *_cnt;
  
  _cnt = calloc(1, sizeof(*_cnt));
  *_cnt = cnt;

  // 1번 스레드를 생성한다.
  err = pthread_create(tid, NULL, thr_fn, (void *)_cnt);
  if (err != 0) {
    printf("오류 발생! 스레드 생성 실패, errno = %d\n", err);

    exit(EXIT_FAILURE);    
  }
  
  // 1번 스레드가 메인 스레드에 결합한다.
  err = pthread_join(*tid, NULL);
  if (err != 0) {
    printf("오류 발생! 스레드 결합 실패, errno = %d\n", err);
    
    exit(EXIT_FAILURE);
  }

  return;
}

int
main(int argc, char *argv[]) {
  pthread_t tid;
  int cnt;

  cnt = 1;
  
  scanf("%d", &N);

  create_thr(&tid, cnt);
  
  exit(EXIT_SUCCESS);
}