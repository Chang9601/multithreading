#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_t tid1, tid2;

void *
thr_fn(void *arg) {
  int duration, cnt, *ret;

  duration = *(int *)arg;
  cnt = 0;

  // 인자가 더 이상 필요하지 않기에 메모리를 해제한다.  
  free(arg);

  // 스레드가 일을 수행하는 데 일정 시간이 걸리는 것처럼 시뮬레이션하는 코드.
  while (cnt < duration) {
    printf("스레드%d %d초간 작업 중.\n", duration, duration);
    sleep(1);
    cnt++;
  }

  // 스레드의 결과는 항상 힙 영역 메모리에 반환되어야 한다.
  ret = calloc(1, sizeof(*ret));
  *ret = duration * duration * duration;

  return (void *)ret;
}

void
create_thr(pthread_t *tid, int duration) {
  int err, *_duration;
  pthread_attr_t attr;
  
  // 생성하는 새로운 스레드의 속성을 기본값으로 지정한다.
  err = pthread_attr_init(&attr);
  if (err != 0) {
    printf("오류 발생! 스레드 속성 설정 실패, errno = %d\n", err);
    
    exit(EXIT_FAILURE);
  }

  // 4번 인자는 반드시 힙 영역 메모리 혹은 데이터 영역 메모리.
  _duration = calloc(1, sizeof(*_duration));
  *_duration = duration;

  // 생성할 스레드가 결합 가능한 스레드인지 아니면 분리된 스레드인지 명시적으로 지정한다.
  // PTHREAD_CREATE_DETACHED: 결합 가능한 스레드.
  // PTHREAD_CREATE_JOINABLE: 분리된 스레드.
  err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  if (err != 0) {
    printf("오류 발생! 스레드 결합 가능 설정 실패, errno = %d\n", err);

    exit(EXIT_FAILURE);   
  }

  err = pthread_create(tid, &attr, thr_fn, (void *)_duration);
  if (err != 0) {
    printf("오류 발생! 스레드 생성 실패, errno = %d\n", err);

    exit(EXIT_FAILURE);
  }

  // pthread_attr_t 구조체를 비활성화한다.
  // pthread_attr_init()의 구현이 속성 객체에 대해 동적 메모리를 할당했다면, pthread_attr_destroy()는 해당 메모리를 해제한다.
  err = pthread_attr_destroy(&attr);
  if (err != 0) {
    printf("오류 발생! 스레드 속성 해제 실패, errno = %d\n", err);

    exit(EXIT_FAILURE);
  }

  return;
}
  
int
main(int argc, char *argv[]) {
  int err;
  // 스레드의 반환값.
  void *thr_ret1, *thr_ret2;

  // 3초간 작업하는 스레드와 8초간 작업하는 스레드를 생성한다.
  create_thr(&tid1, 3);
  create_thr(&tid2, 8);

  printf("3초 작업하는 스레드와 결합 전까지 메인 스레드 블록킹.\n");

  // 메인 스레드가 3초간 작업하는 스레드와 결합할 때까지 대기한다.
  err = pthread_join(tid1, &thr_ret1);
  if (err != 0) {
    printf("오류 발생! 스레드 결합 실패, errno = %d\n", err);

    exit(EXIT_FAILURE);    
  }


  if (thr_ret1) {
    printf("3초 작업하는 스레드의 반환값: %d\n", *(int *)thr_ret1);
    // 3초간 작업하는 스레드의 반환값은 스레드 콜백 함수에서 동적으로 할당된 메모리이기 때문에 해제해야 한다.
    free(thr_ret1);

    thr_ret1 = NULL;
  }

  printf("8초간 작업하는 스레드와 결합 전까지 메인 스레드 블록킹.\n");
  
  // 메인 스레드가 8초간 작업하는 스레드와 결합할 때까지 대기한다.
  err = pthread_join(tid2, &thr_ret2);
  if (err != 0) {
    printf("오류 발생! 스레드 결합 실패, errno = %d\n", err);

    exit(EXIT_FAILURE);    
  }

  if (thr_ret2) {
    printf("8초 작업하는 스레드의 반환값: %d\n", *(int *)thr_ret2);
    // 8초간 작업하는 스레드의 반환값은 스레드 콜백 함수에서 동적으로 할당된 메모리이기 때문에 해제해야 한다.
    free(thr_ret2);

    thr_ret2 = NULL;
  }

  exit(EXIT_SUCCESS);    
}