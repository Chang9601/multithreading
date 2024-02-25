#ifndef _QUEUE_H
#define _QUEUE_H

#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>

#define QUEUE_SIZE 5

struct queue {
  void *elem[QUEUE_SIZE];
  unsigned int front;
  unsigned int rear;
  unsigned int cnt;
  pthread_mutex_t mutex;
  // 조건 변수는 특정 조건이 충족될 때 스레드가 자체적으로 블록되거나 깨어날 수 있도록 한다.
  // 뮤텍스는 조건 기반 블로킹과 스레드 깨우기를 구현할 수 없으며 조건 변수가 필요하다.
  // 조건 변수를 사용하면 스레드가 자원의 상태를 검사하고 원하는 경우 유리한 자원 상태를 기다릴 수 있다.
  // 조건 변수는 상호 배제를 위해 사용되지 않으며 조율(coordination)/신호(signaling)을 위해 사용된다.
  //   pthread_cond_wait():  스레드가 자체를 블록할 수 있다.
  //   pthread_cond_signal(): 스레드가 이미 조건 변수에 의해 블록된 스레드에 재개 신호를 보낼 수 있다.
  // 조건 변수와 뮤텍스의 조합은 고급 스레드 동기화 체계를 구현하는 데 필요하다.
  pthread_cond_t cv;
};

struct queue *
init_queue(void);

void
destroy_queue(struct queue *q);

bool
is_queue_empty(struct queue *q);

bool
is_queue_full(struct queue *q);

bool
enqueue(struct queue *q, void *ptr);

void *
dequeue(struct queue *q);

#endif
