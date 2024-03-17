#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> 
#include <unistd.h>
#include <errno.h>
#include <semaphore.h>

sem_t sems[2];
pthread_t tids[2];

// 영(zero) 세마포어는 0으로 초기화한 세마포어이다.
// 스레드가 영 세마포어에 sem_wait() 함수를 호출하면 호출 스레드가 즉시 블록된다.
// 스레드가 영 세마포어에 sem_post() 함수를 호출하면 세마포어에서 블록된 스레드는 실행을 재개한다.
// 영 세마포어는 두 개의 프로세스/스레드가 번갈아 실행되는 엄격한 교대(strict alternation) 실행을 구현하는데 사용된다.
#define ALLOW_CNT 0

void *
print1_fn(void *arg)
{
  printf("1");
  sem_post(&sems[0]);
  sem_wait(&sems[1]);

  return NULL;
}

void *
print2_fn(void *arg)
{
  sem_wait(&sems[0]);
  printf("2");
  sem_post(&sems[1]);

  return NULL;
}

int
main(int argc, char *argv[])
{
  int i;
  char *thr_names[] = {"스레드1", "스레드2"};

  sem_init(&sems[0], 0, ALLOW_CNT);
  sem_init(&sems[1], 0, ALLOW_CNT);

  pthread_create(&tids[0], NULL, print1_fn, thr_names[0]);
  pthread_create(&tids[1], NULL, print2_fn, thr_names[1]);

  for (i = 0; i < 2; i++)
  {
    pthread_join(tids[i], NULL);
  }

  sem_destroy(&sems[0]);
  sem_destroy(&sems[1]);

  exit(EXIT_SUCCESS);
}