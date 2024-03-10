#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int arr[] = {17, 42, 55, 63, 88};

// 뮤텍스는 여러 동시(concurrent) 스레드가 임계 영역에 동시에 접근할 때 상호 배제를 제공하는 스레드 동기화 도구/구조물이다.
// 즉, 뮤텍스는 공유 자원을 보호하기 위해 임계 영역에 대한 접근을 한 번에 하나의 스레드에만 허용한다.
// 뮤텍스는 기본적으로 공유 자원에 접근하기 전에 설정(잠금)하고 사용이 끝나면 해제(잠금 해제)하는 잠금이다.
// 비유(키, 사물함, 사람)를 들어 설명하자면, 뮤텍스는 키, 임계 영역은 사물함, 스레드를 사물함에 접근하는 사람이다.
// 상호 배제 메커니즘은 스레드가 동일한 데이터 접근 규칙을 따를 때에만 작동한다. 운영체제는 데이터 접근에 대한 직렬화(serialize)를 자동으로 처리하지 않는다. 
// 하나의 스레드가 잠금을 먼저 확보하지 않고 공유 자원에 접근하게 허용하면 다른 스레드가 자원에 접근하기 전에 잠금을 확보하는 경우에도 일관성이 없을 수 있다.

// 코드 잠금 vs. 데이터 잠금
// 코드 잠금은 동시 스레드의 안전하지 않은 접근으로부터 코드 부분을 보호해야 할 경우 사용한다. 뮤텍스가 소스 파일 수준에서 정의된다.
// 데이터 잠금은 동시 스레드의 안전하지 않은 접근으로부터 자료 구조를 보호해야 할 경우 사용한다. 각 자료 구조는 자체 뮤텍스(경호원 역할)와 연관되어 있다.

// 뮤텍스 특성
// 1. 만약 스레드 t가 뮤텍스 m을 설정하면 오직 t만이 뮤텍스 m을 해제할 수 있다.
// 2. 이미 해제된 뮤텍스를 해제할 수 없으며 이를 시도하면 정의되지 않은 동작으로 이어진다.
// 3. 만약 스레드 t1이 뮤텍스 m을 설정하면 다른 스레드(스레드 t2, 스레드 t3 등)가 뮤텍스 m를 설정하려고 뮤텍스 m이 해제할 때까지 블록된다.
// 4. 이미 설정된 뮤텍스 m을 획득하려는 스레드 t2, 스레드 t3 등이 블록되면, 뮤텍스 m이 소유자(스레드 t1)에 의해 해제될 때 뮤텍스 m에 대한 잠금을 얻을 스레드(e.g., 스레드 t2 혹은 스레드 t3)는 운영체제 스케줄링 정책에 따라 결정된다.
// 5. 뮤텍스를 해제할 때 여러 스레드가 블록되어 있다면 잠금에 블록된 모든 스레드가 실행 가능(runnable) 상태로 전환되고 실행 가능한 1번째 스레드가 잠금을 설정할 수 있다. 다른 스레드들은 뮤텍스가 여전히 잠겨 있으므로 다시 사용 가능해질 때까지 대기 상태로 돌아간다. 이렇게 함으로써 한 번에 하나의 스레드만 진행할 수 있다.
// 5. 만약 스레드가 뮤텍스를 두 번 설정하려 하면 자체 교착상태에 빠진다.
// 6. 뮤텍스는 후입선출(LIFO) 순서로 잠금을 해제해야 한다.
// 7. 스레드가 여러 뮤텍스를 잠그는 경우 잠그는 순서가 중요하다.
//    뮤텍스를 잠그는 순서는 유지되어야 한다.
//    잠금 순서는 중요하지 않지만 코드 전체에서 동일한 순서를 따라야 한다.
//    일관되지 않은 잠금 순서는 교착상태로 이어질 수 있다.
// 8. 뮤텍스는 절대로 memcpy() 함수로 복사되어서는 안된다. 정의되지 않은 동작으로 이어진다.

// 두 개의 스레드가 전역 배열에 안전하지 않은 접근을 방지하기 위해 전역 뮤텍스를 사용한다.
// 코드 블록을 잠그는 코드 잠금(code-locking) 뮤텍스.
pthread_mutex_t mutex;

void *
sum_thr_fn(void *arg) 
{
  int i, sum, arr_length;
  
  arr_length = sizeof(arr) / sizeof(arr[0]);

  while (1) 
  {
    sum = 0;
    // 스레드 1번이 배열 요소의 합을 계산할 때 뮤텍스를 잠근다.
    pthread_mutex_lock(&mutex);
    for (i = 0; i < arr_length; i++) 
    {
      sum += arr[i];
    }
    
    // 상호 배제가 보장되므로 합은 항상 265.
    printf("합 = %d\n", sum);
    pthread_mutex_unlock(&mutex);
  }

  return NULL;
}

void *
swap_thr_fn(void *arg) 
{
  int tmp, arr_length;
  
  arr_length = sizeof(arr) / sizeof(arr[0]);

  while (1) 
  {
    // 뮤텍스를 잠그려면 pthread_mutex_lock() 함수를 호출한다. 뮤텍스가 이미 잠겨있는 경우 호출 스레드는 뮤텍스가 잠금 해제될 때까지 블록된다. 
    // 뮤텍스가 잠겨 있는 동안에 스레드 2번이 배열의 두 요소를 교환하려고 하면 뮤텍스가 잠겨 있다는 것을 알게 되어서 스레드 2번은 블록된다.
    pthread_mutex_lock(&mutex);
    tmp = arr[0];
    arr[0] = arr[arr_length - 1];
    arr[arr_length - 1] = tmp;
    // 뮤텍스를 해제하려면 pthread_mutex_unlock() 함수를 호출한다.
    pthread_mutex_unlock(&mutex);
  }

  return NULL;
}

void 
create_sum_thr() 
{
  int err;
  pthread_t tid;

  err = pthread_create(&tid, NULL, sum_thr_fn, NULL);

  if (err != 0) 
  {
    printf("오류 발생! 스레드 생성 실패, errno = %d\n", err);
    exit(EXIT_FAILURE);    
  }

  return;
}

void 
create_swap_thr() 
{
  int err;
  pthread_t tid;

  err = pthread_create(&tid, NULL, swap_thr_fn, NULL);

  if (err != 0) 
  {
    printf("오류 발생! 스레드 생성 실패, errno = %d\n", err);
    exit(EXIT_FAILURE);    
  }

  return;
}


int
main(int args, char *argv[]) 
{
  // 뮤텍스 변수를 사용하기 전에 먼저 상수 PTHREAD_MUTEX_INITIALIZER(정적으로 할당된 뮤텍스에만 해당)로 설정하거나 pthread_mutex_init() 함수를 호출하여 반드시 초기화해야 한다.
  // 뮤텍스를 기본 속성으로 초기화하려면 pthread_mutex_init() 함수의 2번 인자를 NULL로 설정한다.
  pthread_mutex_init(&mutex, NULL);

  create_sum_thr();
  create_swap_thr();

  // 뮤텍스를 동적으로 할당한다면(e.g., malloc() 함수), 메모리를 해제하기 전에 pthread_mutex_destroy() 함수를 호출해야 한다.
  pthread_mutex_destroy(&mutex);
  
  pthread_exit(NULL);

  exit(EXIT_SUCCESS);
}
