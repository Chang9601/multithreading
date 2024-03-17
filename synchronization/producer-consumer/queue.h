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

  // 조건 변수는 스레드에게 제공되는 동기화 메커니즘이다. 이러한 동기화 객체들은 스레드가 만날 수 있는 장소를 제공한다. 
  // 조건 변수는 어떤 시점에 어떤 경쟁 스레드를 블록하거나 재개할지에 대한 결정을 더 세밀하게 제어할 수 있게 해준다.
  // 즉, 조건 변수는 특정 조건이 충족될 때 스레드가 자체적으로 블록되거나 깨어날 수 있도록 한다.
  // 뮤텍스는 조건 기반 블록킹과 스레드 깨우기를 구현할 수 없으며 조건 변수가 필요하다.
  // 뮤텍스는 항상 자원의 속성이지만 조건 변수는 자원의 속성 또는 스레드의 속성일 수 있다.
  // 조건 변수를 사용하면 스레드가 자원의 상태를 검사하고 원하는 경우 유리한 자원 상태를 기다릴 수 있다.
  // 조건 변수와 뮤텍스의 조합은 고급 스레드 동기화 체계를 구현하는 데 필요하다.
  // 뮤텍스와 함께 사용되면 조건 변수는 스레드가 임의의 조건이 발생할 때까지 경쟁없이(race-free) 대기할 수 있게 한다.
  // 조건 자체는 뮤텍스에 의해 보호된다. 스레드는 반드시 먼저 뮤텍스를 잠그고 조건 상태를 변경해야 한다. 
  // 다른 스레드는 조건을 평가하기 위해서는 뮤텍스를 잠그고 있어야 하기 때문에 뮤텍스가 잠겨 있을 때까지 다른 스레드는 변경 사항을 알지 못한다.

  // 자원 조건 변수 vs. 스레드 조건 변수
  // 스레드가 동일한 조건 변수를 공유하는 경우 해당 조건 변수는 자원의 속성이다.
  // 스레드가 각자 고유한 조건 변수를 선택하는 경우 해당 조건 변수는 각 스레드의 속성이다.
  
  // 조건 변수 특성
  // 1. 조건 변수는 뮤텍스처럼 절대로 memcpy() 함수로 복사되어서는 안 된다.
  // 2. pthread_cond_init() 함수를 사용하여 조건 변수를 초기화했다면 pthread_cond_destroy() 함수를 사용하여 조건 변수를 파괴해야 한다.
  // 3. 정적으로 조건 변수를 초기화하기 위해 PTHREAD_COND_INITIALIZER를 사용한다.
  // 4. 조건 변수는 상호 배제를 위해 사용되지 않으며 조율(coordination)/신호(signaling)를 위해 사용된다.
  //      pthread_cond_wait(): 스레드가 자체를 블록할 수 있다.
  //      pthread_cond_signal(): 스레드가 이미 조건 변수에 의해 블록된 스레드에 재개 신호를 보낼 수 있다.
  // 5. 조건 변수는 뮤텍스와 조건부와 관련이 있다. 여러 조건 변수들은 동시에 같은 뮤텍스와 관련될 수 있지만 1개의 조건 변수는 동시에 1개 이상의 뮤텍스와 관련될 수 없다.
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
