#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> // 스레드 사용 시 필요한 헤더.
#include <signal.h>
#include <unistd.h>
#include <errno.h>

// sig_atomic_t 타입은 원자적으로 접근할 수 있는 데이터 타입이다. 즉, 중단되지 않고 쓰여질 수 있는 변수이다.
// 항상 volatile 예약어를 포함시키는데 두 가지 스레드, main() 함수와 비동기적으로 실행되는 시그널 핸들러가 접근하기 때문이다.
// volatile 예약어는 변수가 하드웨어, 인터럽트 또는 동시에 실행되는 스레드와 같은 외부 요소에 의해 변경될 수 있음을 컴파일러에 알려주는 한정자이다. 
// 변수가 volatile로 선언되면 컴파일러는 해당 값에 최적화나 캐싱을 하지 않는데 값이 현재 코드 외부 요소에 의해 변경될 수 있기 때문이다.
volatile sig_atomic_t quit_flag = 0;

static void 
sig_int(int signo)
{
  if (signo == SIGINT)
  {
    quit_flag = 1;
  }
}

void *
thr_fn(void *arg)
{
  char *in;

  in = (char *)arg;

  while (!quit_flag)
  {
    printf("입력 = %s\n", in);
    sleep(1);
  }

  return NULL;
}

void
create_thr()
{
  int err;
  pthread_t tid;
  static char *in = "스레드 #1";
  
  // 스레드 분기 지점.
  // 4번 인자는 반드시 힙 영역 메모리 혹은 데이터 영역 메모리.
  err = pthread_create(&tid, NULL, thr_fn, (void *)in);
  if (err != 0)
  {
    printf("오류 발생! 스레드 생성 실패, errno = %d\n", err);
    exit(EXIT_FAILURE);    
  }

  // 결합 가능 모드 스레드의 자원은 부모 스레드가 해당 스레드에 결합할 때까지 해제되지 않는다.
  // 분리 모드 스레드의 자원은 해당 스레드가 종료되는 즉시 해제된다.
  // 결합 가능 모드 스레드는 실행 중에 분리된 스레드로 변환할 수 있으며 반대도 마찬가지다.
  // 기본적으로 스레드는 결합 가능 모드에서 실행된다.
  // 자원을 해제하는 세 가지 방법.
  // 1. 스레드를 분리 모드로 생성한다(PTHREAD_CREATE_DETACHED).
  // 2. 스레드 생성 후 분리한다(pthread_detach()).
  // 3. 종료된 스레드와 결합한다(pthread_join()).
  // TO-DO: 분리 모드 스레드로 설정 시 메모리 누수 문제 해결하기.
  err = pthread_detach(tid);
  if (err != 0)
  {
    printf("오류 발생! 스레드 분리 실패, errno = %d\n", err);
    exit(EXIT_FAILURE);
  }

  return;
}

int
main(int args, char *argv[])
{
  if (signal(SIGINT, sig_int) == SIG_ERR)
  {
    printf("오류 발생! SIGINT 신호를 받을 수 없음, errno = %d\n", errno);
    exit(EXIT_FAILURE);
  };

  create_thr();
  printf("메인 스레드 종료.\n");
  // pause() 함수가 사용될 경우.
  // printf("메인 스레드 중지.")

  // 프로세스 내에서 어떤 스레드가 exit() 함수, _Exit() 함수 또는 _exit() 함수를 호출하면 전체 프로세스가 종료된다.
  // 마찬가지로 기본 동작이 프로세스를 종료하는 경우 스레드에게 보내진 시그널은 전체 프로세스를 종료시킨다.
  // 단일 스레드는 전체 프로세스를 종료하지 않으면서 제어의 흐름을 중지하는 세 가지 방법으로 종료할 수 있다.
  // 1. 스레드는 시작 루틴(pthread_create() 함수의 인자)에서 간단히 반환할 수 있다. 반환 값은 스레드의 종료 코드이다.
  // 2. 다른 프로세스 내 스레드에 의해 취소될 수 있다.
  // 3. 스레드는 pthread_exit() 함수를 호출할 수 있다.
  pthread_exit(NULL);
  
  // pause() 함수가 주석으로 처리되면 메인 스레드 종료 시 모든 자식 스레드가 종료된다.
  // pause();

  // 프로세스가 종료되는 방법은 총 8가지. 

  // 정상적인 종료는 총 5가지.
  // 1. main() 함수에서 반환.
  // 2. exit() 함수 호출.
  // 3. _exit() 또는 _Exit() 함수 호출.
  // 4. 마지막 스레드의 시작 루틴에서 반환.
  // 5. 마지막 스레드에서 pthread_exit() 함수 호출.

  // 비정상적인 종료는 총 3가지.
  // 1. abort() 함수 호출.
  // 2. 시그널 수신.
  // 3. 마지막 스레드의 취소 요청에 대한 응답.

  // _exit() 함수 및 _Exit() 함수는 즉시 커널로 반환되고, exit() 함수는 일정한 정리(cleanup) 처리를 수행한 후에 커널로 반환된다.
  // main() 함수에서 정수 값을 반환하는 것은 동일한 값을 가지고 exit() 함수를 호출하는 것과 동등하다.
  // 즉, exit(0) == return(0)

  // 해당 코드는 프로세스의 종료에 직접적인 영향을 미치지 않는데 메인 스레드가 자식 스레드를 생성한 직후에 pthread_exit(NULL)을 호출하기 때문이다.
  exit(EXIT_SUCCESS);
}