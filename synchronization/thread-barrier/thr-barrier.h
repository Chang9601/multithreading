#ifndef _THR_BARRIER
#define _THR_BARRIER

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

// 스레드 장벽은 병렬로 작업하는 여러 스레드를 조정하는 데 사용할 수 있는 동기화 메커니즘이다.
// 스레드 장벽은 각 스레드가 협력하는 모든 스레드가 동일한 지점에 도달할 때까지 기다리도록 허용하고 그 후에 그곳에서 계속 실행할 수 있게한다.
// 전체 작업이 진행되기 전에 완료해야 하는 여러 작업을 기다려야 하는 경우 스레드 장벽을 사용할 수 있다.
// pthread_join() 함수는 다른 스레드가 종료될 때까지 하나의 스레드가 대기하도록 일종의 장벽 역할을 한다.
// 스레드 장벽 객체는 이보다 더 일반적인데 스레드 장벽 객체는 임의의 수의 스레드가 모든 스레드의 처리가 완료될 때까지 기다릴 수 있게 하지만, 스레드들은 종료할 필요가 없다. 
// 모든 스레드가 스레드 장벽에 도달한 후에도 작업을 계속할 수 있다.
struct thr_barrier {
  uint32_t threshold_cnt; // 임계치 개수
  uint32_t curr_wait_cnt; // 현재 대기 개수는 스레드 장벽(barrier)에서 블록된 스레드의 수로 값의 범위는 [0, threshold_cnt-1].
  pthread_mutex_t mutex; // 스레드 장벽은 여러 스레드가 공유하는 자료구조이기 때문에 스레드 장벽에 대한 모든 작업을 상호 배제적인 방식으로 수행해야 한다.
  pthread_cond_t cv; // 스레드 장벽의 주요 기능은 스레드를 블록하는 것이므로 스레드가 블록될 조건 변수가 필요하다.
  bool is_ready; // 스레드 장벽 처리가 진행 중인지 여부를 추적하며 기본적으로 true이고 처리가 진행 중일 때는 false로 설정되며 처리가 완료되면 다시 true로 설정된다.
  pthread_cond_t busy_cv; // 스레드 장벽이 처리 중인 경우 즉, 진행 단계에 있는 경우에 어떤 스레드도 스레드 장벽을 사용하지 못하도록하기 위한 조건 변수이다.
};

void
init_thr_barrier(struct thr_barrier *barrier, uint32_t threshold_cnt);

void
wait_thr_barrier(struct thr_barrier *barrier);

void
destroy_thr_barrier(struct thr_barrier *barrier);

void
print_thr_barrier(struct thr_barrier *barrier);

#endif