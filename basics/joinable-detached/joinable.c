#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_t tid1, tid2;

void *
thr_fn(void *arg) 
{
  int duration, cnt, *ret;

  duration = *(int *)arg;
  cnt = 0;

  // 인자가 더 이상 필요하지 않기에 메모리를 해제한다.  
  free(arg);

  // 스레드가 일을 수행하는 데 일정 시간이 걸리는 것처럼 시뮬레이션하는 코드.
  while (cnt < duration) 
  {
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
create_thr(pthread_t *tid, int duration) 
{
  int err, *_duration;
  // pthread_attr_t 구조체를 사용하여 기본 속성을 수정하고 이러한 속성을 생성한 스레드에 연결할 수 있다.
  pthread_attr_t attr;
  
  // pthread_attr_init() 함수를 사용하여 pthread_attr_t 구조체를 초기화한다.
  // pthread_attr_init() 함수를 호출한 후에 pthread_attr_t 구조체에는 구현에서 지원하는 모든 스레드 속성에 대한 기본값이 포함되어 있다.
  err = pthread_attr_init(&attr);
  if (err != 0) 
  {
    printf("오류 발생! 스레드 속성 설정 실패, errno = %d\n", err);
    
    exit(EXIT_FAILURE);
  }

  // 4번 인자는 반드시 힙 영역 메모리 혹은 데이터 영역 메모리.
  _duration = calloc(1, sizeof(*_duration));
  *_duration = duration;

  // 기존 스레드의 종료 상태에 더 이상 관심이 없다면 스레드가 종료될 때 운영체제가 스레드의 자원을 회수할 수 있도록 pthread_detach() 함수를 사용할 수 있다.
  // 스레드를 생성할 때 스레드의 종료 상태가 필요하지 않다고 알고 있다면 pthread_attr_t 구조체에서 detachstate 스레드 속성을 수정하여 스레드가 분리 모드로 시작하도록 할 수 있다. 
  // pthread_attr_setdetachstate() 함수를 사용하여 detachstate 스레드 속성을 두 가지 유효한 값 중 하나로 설정할 수 있다.
  // PTHREAD_CREATE_DETACHED는 스레드를 분리 모드로 시작한다.
  // PTHREAD_CREATE_JOINABLE은 스레드를 결합 가능 모드로 시작하여 애플리케이션에서 종료 상태를 검색할 수 있도록 한다.
  err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  if (err != 0) 
  {
    printf("오류 발생! 스레드 결합 가능 설정 실패, errno = %d\n", err);

    exit(EXIT_FAILURE);   
  }

  err = pthread_create(tid, &attr, thr_fn, (void *)_duration);
  if (err != 0) 
  {
    printf("오류 발생! 스레드 생성 실패, errno = %d\n", err);

    exit(EXIT_FAILURE);
  }

  // pthread_attr_t 구조체를 초기화 해제(deinitialize)하려면 pthread_attr_destroy() 함수를 호출한다.
  // pthread_attr_init() 함수의 구현이 속성 객체에 대해 동적 메모리를 할당한 경우 pthread_attr_destroy() 함수는 해당 메모리를 해제한다.
  // pthread_attr_destroy() 함수는 속성 객체를 잘못 사용할 경우를 대비해 해당 객체를 잘못된 값으로 초기화하므로, 실수로 사용되면 pthread_create() 함수가 오류 코드를 반환한다.
  err = pthread_attr_destroy(&attr);
  if (err != 0) 
  {
    printf("오류 발생! 스레드 속성 해제 실패, errno = %d\n", err);

    exit(EXIT_FAILURE);
  }

  return;
}
  
int
main(int argc, char *argv[]) 
{
  int err;
  // 스레드의 반환값.
  void *thr_ret1, *thr_ret2;

  // 3초간 작업하는 스레드와 8초간 작업하는 스레드를 생성한다.
  create_thr(&tid1, 3);
  create_thr(&tid2, 8);

  printf("3초 작업하는 스레드와 결합 전까지 메인 스레드 블록킹.\n");

  // 호출한 스레드는 지정된 스레드가 pthread_exit() 함수를 호출하거나 시작 루틴에서 반환하거나 취소될 때까지 블록된다.
  // 스레드가 시작 루틴에서 반환한 경우 2번 인자에는 반환 코드가 포함된다. 스레드가 취소된 경우 2번 인자가 가리키는 메모리 위치는 PTHREAD_CANCELED로 설정된다.
  // pthread_join() 함수를 호출하면 결합하는 스레드를 자동으로 분리된 상태로 설정하여 해당 자원을 회수할 수 있다.
  // 스레드가 이미 분리된 상태에 있는 경우 pthread_join() 함수는 실패할 수 있으며 구현에 따라 EINVAL을 반환할 수 있다.
  // 스레드의 반환 값에 관심이 없다면 2번 인자를 NULL로 설정할 수 있다. 이 경우 pthread_join() 함수를 호출하면 지정된 스레드를 기다릴 수 있지만 스레드의 종료 상태를 검색하지 않는다.
  
  // 메인 스레드가 3초간 작업하는 스레드와 결합할 때까지 대기한다.
  err = pthread_join(tid1, &thr_ret1);
  if (err != 0) 
  {
    printf("오류 발생! 스레드 결합 실패, errno = %d\n", err);

    exit(EXIT_FAILURE);    
  }


  if (thr_ret1) 
  {
    printf("3초 작업하는 스레드의 반환값: %d\n", *(int *)thr_ret1);
    // 3초간 작업하는 스레드의 반환값은 스레드 콜백 함수에서 동적으로 할당된 메모리이기 때문에 해제해야 한다.
    free(thr_ret1);

    thr_ret1 = NULL;
  }

  printf("8초간 작업하는 스레드와 결합 전까지 메인 스레드 블록킹.\n");
  
  // 메인 스레드가 8초간 작업하는 스레드와 결합할 때까지 대기한다.
  err = pthread_join(tid2, &thr_ret2);
  if (err != 0) 
  {
    printf("오류 발생! 스레드 결합 실패, errno = %d\n", err);

    exit(EXIT_FAILURE);    
  }

  if (thr_ret2) 
  {
    printf("8초 작업하는 스레드의 반환값: %d\n", *(int *)thr_ret2);
    // 8초간 작업하는 스레드의 반환값은 스레드 콜백 함수에서 동적으로 할당된 메모리이기 때문에 해제해야 한다.
    free(thr_ret2);

    thr_ret2 = NULL;
  }

  exit(EXIT_SUCCESS);    
}