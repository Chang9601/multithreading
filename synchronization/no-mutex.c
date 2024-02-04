#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int arr[] = {17, 42, 55, 63, 88};

static void *
sum_thr_fn(void *arg) {
  int i, sum, arr_length;

  arr_length = sizeof(arr) / sizeof(arr[0]);

  while (1) {
    sum = 0;
    for (i = 0; i < arr_length; i++) {
      sum += arr[i];
    }
    // 가능한 합의 값은 4개
    // 17 + 42 + 55 + 63 + 88 = 265    
    // 17 + 42 + 55 + 63 + 17 = 194
    // 88 + 42 + 55 + 63 + 88 = 336
    // 88 + 42 + 55 + 63 + 17 = 265    
    printf("합 = %d\n", sum);
  }

  return NULL;
}

static void *
swap_thr_fn(void *arg) {
  int tmp, arr_length;
  
  arr_length = sizeof(arr) / sizeof(arr[0]);

  while (1) {
    tmp = arr[0];
    arr[0] = arr[arr_length - 1];
    arr[arr_length - 1] = tmp;
  }

  return NULL;
}

void 
create_sum_thr() {
  pthread_t tid;
  int err;

  err = pthread_create(&tid, NULL, sum_thr_fn, NULL);

  if (err != 0) {
    printf("오류 발생! 스레드 생성 실패, errno = %d\n", err);

    exit(EXIT_FAILURE);    
  }
}

void 
create_swap_thr() {
  pthread_t tid;
  int err;
  
  err = pthread_create(&tid, NULL, swap_thr_fn, NULL);

  if (err != 0) {
    printf("오류 발생! 스레드 생성 실패, errno = %d\n", err);

    exit(EXIT_FAILURE);
  }
}

int
main(int args, char *argv[]) {
  create_sum_thr();
  create_swap_thr();

  pthread_exit(NULL);

  exit(EXIT_SUCCESS);
}