#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_t tid1;
pthread_t tid2;

void *
thr_fn(void *arg) {
  int id = *(int *)arg;
  // 인자가 더 이상 필요하지 않기에 메모리를 해제한다.
  free(arg);

  int count = 0;
  // 스레드가 일을 수행하는 데 일정 시간이 걸리는 것처럼 시뮬레이션하는 코드
  while (count < id) {
    printf("스레드%d 작업 중\n", id);
    sleep(1);
    count++;
  }

  // 스레드의 결과는 항상 힙 메모리에 반환되어야 한다.
  int *result = calloc(1, sizeof(*result));
  *result = id * id * id;

  return (void *)result;
}

void
create_thr(pthread_t *tid, int id) {
  pthread_attr_t attr;
  // 생성하는 새로운 스레드의 속성을 지정한다.
  pthread_attr_init(&attr);

  // 4번째 인자는 반드시 힙 영역 혹은 데이터 영역 메모리
  int *_id = calloc(1, sizeof(*_id));
  *_id = id;

  // 생성할 스레드가 결합 가능한 스레드인지 아니면 분리된 스레드인지 명시적으로 지정한다.
  // PTHREAD_CREATE_DETACHED: 결합 가능한 스레드
  // PTHREAD_CREATE_JOINABLE: 분리된 스레드
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  int err = pthread_create(tid, &attr, thr_fn, (void *)_id);
  
  if (err != 0) {
    printf("오류 발생! 스레드 생성 실패, errno = %d\n", err);
    exit(EXIT_FAILURE);    
  }

  pthread_attr_destroy(&attr);
}
  
int
main(int argc, char *argv[]) {
  // 스레드의 반환값
  void *thr_ret1;
  void *thr_ret2;

  // 3초, 8초간 작업하는 스레드를 생성한다.
  create_thr(&tid1, 3);
  create_thr(&tid2, 8);

  printf("스레드1 결합 전까지 메인 스레드 블록킹\n");

  // 메인 스레드가 스레드1이 합류할 때까지 대기한다.
  pthread_join(tid1, &thr_ret1);

  if (thr_ret1) {
    printf("스레드1의 반환값 = %d\n", *(int *)thr_ret1);
    // 스레드1의 반환값은 스레드 콜백 함수에서 동적으로 할당된 메모리이기 때문에 해제해야 한다.
    free(thr_ret1);
    thr_ret1 = NULL;
  }

  printf("스레드2 결합 전까지 메인 스레드 블록킹\n");
  
  // 메인 스레드가 스레드2가 합류할 때까지 대기한다.
  pthread_join(tid2, &thr_ret2);

  if (thr_ret2) {
    printf("스레드2의 반환값 = %d\n", *(int *)thr_ret2);
    free(thr_ret2);
    thr_ret2 = NULL;
  }

  exit(EXIT_SUCCESS);    
}