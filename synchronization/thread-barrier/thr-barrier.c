#include "thr-barrier.h"

void
init_thr_barrier(struct thr_barrier *barrier, uint32_t threshold_cnt)
{
  barrier->threshold_cnt = threshold_cnt;
  barrier->curr_wait_cnt = 0;
  barrier->is_ready = true;

  pthread_mutex_init(&barrier->mutex, NULL);
  pthread_cond_init(&barrier->cv, NULL);
  pthread_cond_init(&barrier->busy_cv, NULL);
}

void
destroy_thr_barrier(struct thr_barrier *barrier)
{

  pthread_mutex_destroy(&barrier->mutex);
  pthread_cond_destroy(&barrier->cv);
  pthread_cond_destroy(&barrier->busy_cv);
}

void
wait_thr_barrier(struct thr_barrier *barrier)
{
  pthread_mutex_lock(&barrier->mutex);

  // 스레드 장벽이 처리 단계에 있으며 스레드들이 스레드 장벽 지점을 넘어 실행을 재개 중이며 재개하지 않은 스레드가 있다는 것을 의미한다.
  // 다시말해,스레드 장벽은 현재 바쁜 상태로 스레드 장벽이 사용 불가능하며 스레드 장벽 상태를 검사하는 스레드(즉, API를 호출한 스레드)가 조건 변수에 의해 블록되어야 한다.
  // 의도하지 않은 기상을 방지하기 위해 while 문을 사용한다.
  while (barrier->is_ready == false)
  {
    pthread_cond_wait(&barrier->busy_cv, &barrier->mutex);
  }

  // 스레드 장벽을 사용할 수 있으며 현재 스레드가 장벽 지점에 도달했을 현재 스레드가 마지막 스레드라면 스레드 장벽 상태를 갱신하여 스레드 장벽이 처리 단계에 있다는 것을 나타낸다.
  // 스레드 장벽은 블록된 모든 스레드를 해제해야 하며 마지막 스레드는 신호를 생성해야 한다.
  // 신호는 스레드 장벽에 블록된 다른 스레드에 의해 소비되며 현재 스레드는 단순히 스레드 장벽 뮤텍스의 잠금을 해제하고 종료해야 한다.
  if (barrier->curr_wait_cnt + 1 == barrier->threshold_cnt)
  {
    barrier->is_ready = false;
    pthread_cond_signal(&barrier->cv);
    pthread_mutex_unlock(&barrier->mutex);

    return;
  }

  // 스레드 장벽을 사용할 수 있으며 현재 스레드가 장벽 지점에 도달했을 현재 스레드가 마지막 스레드가 아니라면 스레드 장벽에 블록된 스레드 수를 추적하는 개수를 증가시킨다.
  // 현재 스레드는 스레드 장벽 지점에서 블록된다.
  barrier->curr_wait_cnt++;
  pthread_cond_wait(&barrier->cv, &barrier->mutex);

  // 다음 코드는 블록된 스레드가 신호를 받을 때만 실행된다.
  // 즉, 스레드 장벽 지점에서 블록된 스레드 중 하나가 신호를 받는다.
  // 스레드 중 하나가 실행을 시작하기에 스레드 장벽 지점에서 블록된 전체 스레드 수가 하나 줄어든다.
  barrier->curr_wait_cnt--;

  // 스레드 장벽 지점을 떠나는 스레드가 마지막 스레드인지 아닌지 다뤄야 한다.
  // 만약 마지막 스레드가 아니라면, 그 스레드는 신호를 생성해야 한다. 즉, 스레드 장벽 지점에서 블록된 다른 스레드에 대한 신호를 생성한다.
  // 만약 마지막 스레드라면, 해당 스레드는 신호를 생성해서는 안 된다. 이 경우에는 스레드 장벽의 처리 단계를 완료로 표시(is_ready = true)하고 스레드 장벽의 조건 변수를 브로드캐스트해야 한다.
  // 스레드 장벽의 처리 단계가 진행 중일 때 여러 스레드가 있을 수 있기 때문이다. 
  // 따라서 마지막 스레드가 스레드 장벽 지점을 떠날 때, 스레드 장벽 조건 변수에 블록된 모든 스레드에게 스레드 장벽이 사용 가능하다고 알려주는 것은 마지막 스레드의 책임이다.

  if (barrier->curr_wait_cnt == 0)
  {
    barrier->is_ready = true;
    pthread_cond_broadcast(&barrier->busy_cv);
  }
  else
  {
    pthread_cond_signal(&barrier->cv);
  }

  pthread_mutex_unlock(&barrier->mutex);
}

void
print_thr_barrier(struct thr_barrier *barrier)
{
  printf("임계치 개수: %u\n", barrier->threshold_cnt);
  printf("현재 대기 개수: %u\n", barrier->curr_wait_cnt);
  printf("스레드 장벽의 처리 상태: %s\n", barrier->is_ready ? "완료" : "진행");
}