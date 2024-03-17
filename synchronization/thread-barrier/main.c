#include <stdio.h>
#include <stdlib.h>
#include "thr-barrier.h"

struct thr_barrier barrier;
pthread_t tids[3];

void *
thr_fn(void *arg)
{
  // 스레드가 스레드 장벽에서 나가는 순서는 무작위일 수 있으며 이는 중요하지 않다.
  // 중요한 것은 모든 스레드가 1번 스레드 장벽을 통과한 다음 모든 스레드가 2번 스레드 장벽을 통과하고 즉, 통과하는 순서이다.
  char *thr_name = (char *)arg;

  // 모든 스레드가 스레드 장벽에 블록된다.
  // 즉, 스레드가 모두 스레드 장벽 지점에 도달할 때까지 블록된다.
  wait_thr_barrier(&barrier);
  // 블록된 스레드의 개수만큼 출력문이 출력된다.
  printf("%s가 1번 스레드 장벽을 치운다.\n", thr_name);

  wait_thr_barrier(&barrier);
  printf("%s가 2번 스레드 장벽을 치운다.\n", thr_name);

  wait_thr_barrier(&barrier);
  printf("%s가 3번 스레드 장벽을 치운다.\n", thr_name);

  return NULL;
}

int 
main(int argc, char *argv[])
{
  const char *thr1_name = "스레드1";
  const char *thr2_name = "스레드2";
  const char *thr3_name = "스레드3";

  init_thr_barrier(&barrier, 3);

	pthread_create(&tids[0], NULL, thr_fn, (void *)thr1_name);
	pthread_create(&tids[1], NULL, thr_fn, (void *)thr2_name);
	pthread_create(&tids[2], NULL, thr_fn, (void *)thr3_name);

  pthread_join(tids[0], NULL);
  pthread_join(tids[1], NULL);
  pthread_join(tids[2], NULL);

  print_thr_barrier(&barrier);

  exit(EXIT_SUCCESS);
}