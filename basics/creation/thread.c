#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> // 스레드 사용 시 필요한 헤더
#include <unistd.h>

static void *
thr_fn(void *arg) {

  char *in = (char *)arg;
  int cnt = 1;

  while (cnt < 20) {
    printf("입력 = %s\n", in);
    sleep(1);

  if (cnt == 10) {
    pthread_exit((void *)0);
  }

    cnt++;
  }
}

void
create_thr() {
  pthread_t tid;
  
  static char *in = "스레드 #1";
  
  // 스레드 분기 지점
  // 4번째 인자는 반드시 힙 영역 혹은 데이터 영역 메모리
  int err = pthread_create(&tid, NULL, thr_fn, (void *)in);

  if (err != 0) {
    printf("오류 발생! 스레드 생성 실패, errno = %d\n", err);
    exit(EXIT_FAILURE);    
  }

  // 스레드의 자원은 종료 시 즉시 해제되지 않는다. 
  // 단, 스레드가 PTHREAD_CREATE_DETACHED로 설정된 분리된 상태 속성으로 생성되었거나 pthread_detach()가 해당 pthread_t에 대해 호출된 경우에는 스레드의 자원이 해제된다.
  // 분리되지 않은 스레드는 pthread_join() 또는 pthread_detach()에 해당하는 스레드 식별자가 전달될 때까지 종료된 상태로 유지된다.
  // 자원을 해제하는 세 가지 방법
  //   1. 스레드를 분리된 속성으로 생성한다(PTHREAD_CREATE_DETACHED).
  //   2. 스레드 생성 후 분리한다(pthread_detach()).
  //   3. 종료된 스레드와 결합한다(pthread_join()).
  pthread_join(tid, NULL);
}

int
main(int args, char *argv[]) {

  create_thr();
  printf("메인 스레드 중지\n");
  // 메인 스레드가 종료되어도 자식 스레드는 실행 중이다.
  // create_thr() 함수에서 pthread_join() 함수가 사용될 경우 필요없다.
  pthread_exit((void *)0);
  // pause(); pause() 함수가 주석으로 처리되면 메인 스레드 종료 시 모든 자식 스레드가 종료된다.

  exit(EXIT_SUCCESS);
}
