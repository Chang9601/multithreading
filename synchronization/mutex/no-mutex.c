#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int arr[] = {17, 42, 55, 63, 88};

static void *
sum_thr_fn(void *arg)
{
  int i, sum, arr_length;

  arr_length = sizeof(arr) / sizeof(arr[0]);

  while (1) 
  {
    sum = 0;
    // 여러 스레드 간의 공유 자원에 대한 동시 접근 때문에 데이터가 일관성을 잃을 수 있다. 
    // 즉, 근본 원인은 동시성(concurrency)이다.
    // 여러 스레드에 의해 공유 자원이 접근되는 코드 영역을 임계 영역이라고 한다.
    // 임계 영역은 동시에 실행되어야 하지만 한 번에 하나의 스레드에 의해서만 실행되어야 한다. 
    // 그렇지 않으면, 예상치 못한 동작, 세그멘테이션 폴트, 데이터 손상, 비정상적인 동작이 발생한다.
    // 스레드 동기화는 다중 스레드 프로그램에서 여러 스레드가 공유 자원에 대해 충돌하는 작업(읽기-쓰기 또는 쓰기-쓰기)를 수행할 때 필요하다.
    // 즉, 스레드 동기화는 임계 영역을 식별하고 여러 스레드에 의한 프로세스의 공유 자원에 대한 보호되지 않은 동시 접근을 방지하기 위해 여러 기술을 적용한다.
    for (i = 0; i < arr_length; i++) 
    {
      sum += arr[i];
    }
    // 가능한 합의 값은 4개.
    // 17 + 42 + 55 + 63 + 88 = 265    
    // 17 + 42 + 55 + 63 + 17 = 194
    // 88 + 42 + 55 + 63 + 88 = 336
    // 88 + 42 + 55 + 63 + 17 = 265    
    printf("합 = %d\n", sum);
  }

  return NULL;
}

static void *
swap_thr_fn(void *arg) 
{
  int tmp, arr_length;
  
  arr_length = sizeof(arr) / sizeof(arr[0]);

  while (1) 
  {
    tmp = arr[0];
    arr[0] = arr[arr_length - 1];
    arr[arr_length - 1] = tmp;
  }

  return NULL;
}

void 
create_sum_thr() 
{
  pthread_t tid;
  int err;

  err = pthread_create(&tid, NULL, sum_thr_fn, NULL);

  if (err != 0) 
  {
    printf("오류 발생! 스레드 생성 실패, errno = %d\n", err);

    exit(EXIT_FAILURE);    
  }
}

void 
create_swap_thr() 
{
  pthread_t tid;
  int err;
  
  err = pthread_create(&tid, NULL, swap_thr_fn, NULL);

  if (err != 0) 
  {
    printf("오류 발생! 스레드 생성 실패, errno = %d\n", err);

    exit(EXIT_FAILURE);
  }
}

int
main(int args, char *argv[]) 
{
  create_sum_thr();
  create_swap_thr();

  pthread_exit(NULL);

  exit(EXIT_SUCCESS);
}