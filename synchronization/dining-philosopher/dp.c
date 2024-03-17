
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include "dp.h"

#define N_PHIL 5
#define N_SP 5

struct phil phils[N_PHIL];
struct sp sps[N_SP];

// ... 철학자 4번 숟가락 4번 철학자 0번 숟가락 0번 철학자 1번 숟가락 1번 ...
struct sp *
get_left_sp(struct phil *phil)
{
  int id = phil->id;

  return &sps[id];
}

struct sp *
get_right_sp(struct phil *phil)
{
  int id = phil->id;
  
  if (id == 0)
  {
    return &sps[N_SP - 1];
  }

  return &sps[id - 1];
}

void
eat(struct phil *phil)
{
  struct sp *left_sp = get_left_sp(phil);
  struct sp *right_sp = get_right_sp(phil);

  // 철학자는 왼쪽 숟가락과 오른쪽 숟가락 둘 다 차지하고 있어야 케이크를 먹을 수 있다.
  assert(left_sp->phil == phil);
  assert(left_sp->is_taken == true);

  assert(right_sp->phil == phil);
  assert(right_sp->is_taken == true);
  
  phil->cnt++;

  printf("철학자 %d가 왼쪽 숟가락 %d과 오른쪽 숟가락 %d으로 케이크를 %d번 먹는다.\n", phil->id, left_sp->id, right_sp->id, phil->cnt);

  // 1초 동안 케이크를 먹는다.
  sleep(1);
}

bool
hold_both_sps(struct phil *phil)
{
  struct sp *left_sp = get_left_sp(phil);
  struct sp *right_sp = get_right_sp(phil);

  // 왼쪽 숟가락의 상태를 확인하기 전에 왼쪽 숟가락의 뮤텍스를 잠근다.
  // 철학자가 왼쪽 숟가락의 상태를 점검하는 동안 다른 철학자는 이를 변경해서는 안 되기 때문이다.
  printf("철학자 %d가 왼쪽 숟가락 %d의 뮤텍스를 잠그려고 대기 중이다.\n", phil->id, left_sp->id);
  pthread_mutex_lock(&left_sp->mutex);

  printf("철학자 %d가 왼쪽 숟가락 %d의 상태를 점검한다.\n", phil->id, left_sp->id);

  // 왼쪽 숟가락을 사용할 수 없으면 대기한다.
  while (left_sp->is_taken && left_sp->phil != phil)
  {
    printf("철학자 %d는 왼쪽 숟가락 %d을 사용할 수 없어서 블록한다.\n", phil->id, left_sp->id);
    pthread_cond_wait(&left_sp->cv, &left_sp->mutex);
    printf("철학자 %d는 왼쪽 숟가락 %d을 잡을 수 있다는 신호를 받는다.\n", phil->id, left_sp->id);
  }

  // 왼쪽 숟가락을 사용할 수 있기에 왼쪽 숟가락을 잡고 오른쪽 숟가락을 시도한다.
  printf("철학자 %d는 왼쪽 숟가락 %d을 사용할 수 있어서 잡으려고 한다.\n", phil->id, left_sp->id);
  left_sp->phil = phil;
  left_sp->is_taken = true;
  
  printf("철학자 %d는 왼쪽 숟가락 %d을 잡는다.\n", phil->id, left_sp->id);
  pthread_mutex_unlock(&left_sp->mutex);

  // 오른쪽 숟가락의 상태를 확인하기 전에 숟가락의 뮤텍스를 잠근다.
  // 철학자가 오른쪽 숟가락의 상태를 점검하는 동안 다른 철학자는 이를 변경해서는 안 되기 때문이다.
  printf("철학자 %d가 오른쪽 숟가락 %d의 뮤텍스를 잠그려고 대기 중이다.\n", phil->id, right_sp->id);
  pthread_mutex_lock(&right_sp->mutex);

  printf("철학자 %d가 오른쪽 숟가락 %d의 상태를 점검한다.\n", phil->id, right_sp->id);

  // 오른쪽 숟가락을 사용할 수 있어서 잡는다.
  if (!right_sp->is_taken)
  {
    right_sp->phil = phil;
    right_sp->is_taken = true;

    pthread_mutex_unlock(&right_sp->mutex);

    return true;
  }
  // 오른쪽 숟가락을 사용할 수 없어서 왼쪽 숟가락도 놓는다.
  else
  {
    if (right_sp->phil != phil)
    {
      printf("철학자 %d는 오른쪽 숟가락 %d을 다른 철학자 %d가 사용하고 있다는 것을 알고 왼쪽 숟가락도 놓는다.\n", phil->id, right_sp->id, right_sp->phil->id);
      pthread_mutex_lock(&left_sp->mutex);

      assert(left_sp->phil == phil);
      assert(left_sp->is_taken == true);

      left_sp->phil = NULL;
      left_sp->is_taken = false;

      printf("철학자 %d는 왼쪽 숟가락 %d을 놓는다.\n", phil->id, left_sp->id);
      pthread_mutex_unlock(&left_sp->mutex);
      pthread_mutex_unlock(&right_sp->mutex);

      return false;
    }
    // 해당 코드 블록을 사용되지 않을 가능성이 높지만 모든 시나리오를 다루기 위해 완성한다.
    else
    {
      printf("철학자 %d는 이미 오른쪽 숟가락 %d를 가지고 있다.\n", phil->id, right_sp->id);
      pthread_mutex_unlock(&right_sp->mutex);

      return true;
    }
  }

  return false;
}

void
release_both_sps(struct phil *phil)
{
  struct sp *left_sp = get_left_sp(phil);
  struct sp *right_sp = get_right_sp(phil);

  pthread_mutex_lock(&left_sp->mutex);
  pthread_mutex_lock(&right_sp->mutex);

  assert(left_sp->phil == phil);
  assert(left_sp->is_taken == true);

  assert(right_sp->phil == phil);
  assert(right_sp->is_taken == true);

  printf("철학자 %d가 왼쪽 숟가락 %d을 놓는다.\n", phil->id, left_sp->id);
  left_sp->is_taken = false;
  left_sp->phil = NULL;

  // 현재 철학자 왼쪽에 앉아있는 철학자가 현재 철학자의 왼쪽 숟가락(즉, 왼쪽 철학자의 오른쪽 숟가락)을 잡기위해 기다리고 있을 수 있다.
  // 따라서 현재 철학자는 방금 놓은 왼쪽 숟가락의 조건 변수에 대한 신호를 보내야 한다.
  pthread_cond_signal(&left_sp->cv);
  printf("철학자 %d가 왼쪽 숟가락 %d을 기다리는 철학자에게 신호를 보낸다.\n", phil->id, left_sp->id);

  pthread_mutex_unlock(&left_sp->mutex);

  printf("철학자 %d가 오른쪽 숟가락 %d을 놓는다.\n", phil->id, right_sp->id);
  right_sp->is_taken = false;
  right_sp->phil = NULL;

  // 현재 철학자 오른쪽에 앉아있는 철학자가 현재 철학자의 왼쪽 숟가락(즉, 오른쪽 철학자의 왼쪽 숟가락)을 잡기위해 기다리고 있을 수 있다.
  // 따라서 현재 철학자는 방금 놓은 오른쪽 숟가락의 조건 변수에 대한 신호
  pthread_cond_signal(&right_sp->cv);
  printf("철학자 %d가 오른쪽 숟가락 %d을 기다리는 철학자에게 신호를 보낸다.\n", phil->id, right_sp->id);

  pthread_mutex_unlock(&right_sp->mutex);
}

void *
phil_fn(void *arg)
{
  struct phil *phil = (struct phil *)arg;

  while (1)
  {
    if (hold_both_sps(phil))
    { 
      eat(phil);
      release_both_sps(phil);
      // 케이크를 먹은 뒤 숟가락을 잡기 전에 1초 대기한다.
      sleep(1);
    }
  }

  return NULL;
}

int
main(int argc, char *argv[])
{
  int i;
  pthread_attr_t attr;

  for (i = 0; i < N_SP; i++)
  {
    sps[i].id = i;
    sps[i].is_taken = false;
    sps[i].phil = NULL;
    pthread_mutex_init(&sps[i].mutex, NULL);
    pthread_cond_init(&sps[i].cv, NULL);
  }

  pthread_attr_init(&attr);
  // 모든 스레드가 무한 루프에서 실행될 것이기 때문에 분리 모드로 생성한다.
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  for (i = 0; i < N_PHIL; i++)
  {
    phils[i].id = i;
    phils[i].cnt = 0;
    pthread_create(&phils[i].tid, &attr, phil_fn, (void *)&phils[i]);
  }

  pthread_attr_destroy(&attr);
  
  pthread_exit(NULL);

  exit(EXIT_SUCCESS);
}