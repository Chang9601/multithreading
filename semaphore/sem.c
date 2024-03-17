#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> 
#include <unistd.h>
#include <errno.h>
#include <semaphore.h>

// 세마포어는 여러 프로세스/스레드에 대한 공유 자원에 접근을 제공하기 위해 사용되는 계수기이다.
// 공유 자원을 얻기 위해 프로세스/스레드는 다음을 수행한다.
// 1. 자원을 제어하는 세마포어를 테스트한다.
// 2. 세마포어의 값이 양수인 경우 프로세스/스레드는 자원을 사용할 수 있다. 프로세스/스레드는 세마포어 값을 1 감소시킨다. 이는 프로세스/스레드가 자원의 한 단위를 사용했음을 나타낸다.
// 3. 세마포어의 값이 0인 경우, 프로세스/스레드는 세마포어 값이 0보다 클 때까지 대기하고 프로세스/스레드가 깨어나면 단계 1로 돌아간다.
// 세마포어를 올바르게 구현하려면 세마포어 값의 테스트 및 값을 감소시키는 것이 원자적인 작업이어야 한다.
// 일반적으로 세마포어는 양수로 초기화할 수 있다.
// 1로 초기화되는 이진 세마포어로 이는 단일 자원을 제어하며 뮤텍스와 동일한다.
// 하지만 뮤텍스와 달리 세마포어는 다른 프로세스/스레드에 의해 블록이 해제될 수 있다.
sem_t sem;
pthread_t tids[5];

#define ALLOW_CNT 2

void *
thr_fn(void *arg)
{
  int i;
	char *thr_name;

  thr_name = (char *)arg;

  // 세마포어의 값을 무조건 감소시킨다.
  // 감소 후 세마포어의 값이 0보다 작으면 호출한 스레드를 블록한다.
  sem_wait(&sem);
  printf("%s가 임계 영역에 들어갔다.\n", thr_name);

  for (i = 0; i < 5; i++)
  {
    printf("임계 영역에서 %s 실행 중.\n", thr_name);
    sleep(1);
  }

  // 세마포어의 값을 무조건 증가시킨다.
  // sem_wait() 함수에서 블록된 스레드가 있다면 해당 스레드에 신호를 보낸다.
  sem_post(&sem);
  printf("%s가 임계 영역을 나갔다.\n", thr_name);

  return NULL;
}

void
create_thr(pthread_t *tid, void *arg)
{
  pthread_create(tid, NULL, thr_fn, arg);
}

int
main(int argc, char *argv[])
{
  int i;
  char *thr_names[] = {"스레드1", "스레드2", "스레드3", "스레드4", "스레드5"};

  // 2번 인자는 세마포어를 여러 프로세스와 함께 사용할 것인지를 나타내며 그럴 경우 0이 아닌 값으로 설정한다. 
  // 3번 인자는 세마포어의 초기값을 지정한다.
  sem_init(&sem, 0, ALLOW_CNT);

  for (i = 0; i < 5; i++)
  {
    create_thr(&tids[i], thr_names[i]);
  }

  for (i = 0; i < 5; i++)
  {
    pthread_join(tids[i], NULL);
  }

  // sem_destroy() 를 호출한 후에는 인자와 관련된 어떤 세마포어 함수도 사용할 수 없으며 세마포어를 다시 초기화하려면 sem_init()를 호출해야 한다.
  sem_destroy(&sem);

  exit(EXIT_SUCCESS);
}