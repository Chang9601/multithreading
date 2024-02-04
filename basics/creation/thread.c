#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> // 스레드 사용 시 필요한 헤더.
#include <signal.h>
#include <unistd.h>
#include <errno.h>

// 원자적으로 접근할 수 있는 데이터 타입. 즉, 중단되지 않고 쓰여질 수 있는 변수.
// 항상 volatile 예약어를 포함시키는데 두 가지 스레드, 메인 함수와 비동기적으로 실행되는 시그널 핸들러가 접근하기 때문이다.
// volatile 예약어는 변수가 하드웨어, 인터럽트 또는 동시에 실행되는 스레드와 같은 외부 요소에 의해 변경될 수 있음을 컴파일러에 알려주는 한정자. 
// 변수가 volatile로 선언되면 컴파일러는 해당 값에 최적화나 캐싱을 하지 않는데 값이 현재 코드 외부 요소에 의해 변경될 수 있기 때문이다.
volatile sig_atomic_t quit_flag = 0;

static void 
sig_int(int signo) {
  if (signo == SIGINT) {
    quit_flag = 1;
  }
}

static void *
thr_fn(void *arg) {
  char *in;

  in = (char *)arg;

  while (!quit_flag) {
    printf("입력 = %s\n", in);
    sleep(1);
  }

  return NULL;
}

void
create_thr() {
  int err;
  pthread_t tid;
  static char *in = "스레드 #1";
  
  // 스레드 분기 지점.
  // 4번 인자는 반드시 힙 영역 혹은 데이터 영역 메모리.
  err = pthread_create(&tid, NULL, thr_fn, (void *)in);
  if (err != 0) {
    printf("오류 발생! 스레드 생성 실패, errno = %d\n", err);
    exit(EXIT_FAILURE);    
  }

  // 결합 가능한 스레드의 자원은 부모 스레드가 해당 스레드에 결합할 때까지 해제되지 않는다.
  // 분리된 스레드의 자원은 해당 스레드가 종료되는 즉시 해제된다.
  // 결합 가능한 스레드는 실행 중에 분리된 스레드로 변환할 수 있으며 반대도 마찬가지다.
  // 기본적으로 스레드는 결합 가능한 모드에서 실행된다.
  // 자원을 해제하는 세 가지 방법.
  //   1. 스레드를 분리된 속성으로 생성한다(PTHREAD_CREATE_DETACHED).
  //   2. 스레드 생성 후 분리한다(pthread_detach()).
  //   3. 종료된 스레드와 결합한다(pthread_join()).
  // TO-DO: 분리된 스레드로 설정 시 메모리 누수 문제 해결하기.
  err = pthread_detach(tid);
  if (err != 0) {
    printf("오류 발생! 스레드 분리 실패, errno = %d\n", err);
    exit(EXIT_FAILURE);
  }

  return;
}

int
main(int args, char *argv[]) {
  if (signal(SIGINT, sig_int) == SIG_ERR) {
    printf("오류 발생! SIGINT 신호를 받을 수 없음, errno = %d\n", errno);
    exit(EXIT_FAILURE);
  };

  create_thr();
  printf("메인 스레드 종료.\n");
  // pause() 함수가 사용될 경우.
  // printf("메인 스레드 중지.")

  // 메인 스레드가 종료되어도 전체 프로세스를 종료하지 않으며 자식 스레드는 계속 실행된다.
  // create_thr() 함수에서 pthread_join() 함수가 사용될 경우 필요없다.
  pthread_exit(NULL);
  
  // pause() 함수가 주석으로 처리되면 메인 스레드 종료 시 모든 자식 스레드가 종료된다.
  // pause();

  // 메인 함수에서 return을 사용하거나 exit() 함수를 호출하면 프로세스를 종료시킨다.
  exit(EXIT_SUCCESS);
}