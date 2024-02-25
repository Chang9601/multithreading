#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int arr[] = {17, 42, 55, 63, 88};
// 두 개의 스레드가 전역 배열에 안전하지 않은 접근을 방지하기 위해 전역 뮤텍스를 사용한다.
// 코드 블록을 잠그는 코드 잠금(code-locking) 뮤텍스
pthread_mutex_t mutex;

static void *
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
    
    // 상호 배제가 보장되므로 합은 항상 265
    printf("합 = %d\n", sum);
    pthread_mutex_unlock(&mutex);
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
    // 뮤텍스가 잠겨 있는 동안에 스레드 2번이 배열의 두 요소를 교환하려고 하면 뮤텍스가 잠겨 있다는 것을 알게 되어서 스레드 2번은 블록된다.
    pthread_mutex_lock(&mutex);
    tmp = arr[0];
    arr[0] = arr[arr_length - 1];
    arr[arr_length - 1] = tmp;
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
  // 프로세스가 시작되면 메인 함수에서 뮤텍스를 초기화한다.
  pthread_mutex_init(&mutex, NULL);

  create_sum_thr();
  create_swap_thr();

  pthread_exit(NULL);

  exit(EXIT_SUCCESS);
}
