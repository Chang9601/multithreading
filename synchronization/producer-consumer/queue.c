#include "queue.h"

struct queue *
init_queue(void) 
{
  struct queue *q;
  
  q = calloc(1, sizeof(*q));
  q->front = q->rear = q->cnt = 0;

  pthread_mutex_init(&q->mutex, NULL);
  // 조건 변수를 사용하기 전에는 먼저 초기화해야 한다.
  // 정적으로 할당된 조건 변수에는 상수 PTHREAD_COND_INITIALIZER를 할당한다.
  // 동적으로 할당된 조건 변수에는 pthread_cond_init 함수를 사용하여 초기화 한다.
  // 조건 변수를 기본 속성으로 생성해야 하는 경우가 아니라면 pthread_cond_init() 함수의 2번 인자는 NULL로 설정할 수 있다.
  pthread_cond_init(&q->cv, NULL);

  return q;
}

void
destroy_queue(struct queue *q)
{
  pthread_mutex_destroy(&q->mutex);
  // 조건 변수의 기본 메모리를 해제하기 전에 조건 변수를 비초기화하려면(deinitialize) pthread_cond_destroy() 함수를 사용한다.
  pthread_cond_destroy(&q->cv);

  free(q);
}

bool
is_queue_empty(struct queue *q) 
{
  return q->cnt <= 0;
}

bool
is_queue_full(struct queue *q) 
{
  return q->cnt >= QUEUE_SIZE;
}

bool
enqueue(struct queue *q, void *ptr) 
{
  if (!q || !ptr) 
  {
    return false;
  }

  if (is_queue_full(q)) 
  {
    return false;
  }

  if (is_queue_empty(q)) 
  {
    q->elem[q->front] = ptr;
    q->cnt++;

    return true;
  }

  if (q->rear == QUEUE_SIZE - 1) 
  {
    q->rear = 0;
  }
  else 
  {
    q->rear++;
  }

  q->elem[q->rear] = ptr;
  q->cnt++;

  return true;
}

void *
dequeue(struct queue *q) 
{
  void *elem;
  
  elem = NULL;

  if (!q) 
  {
    return NULL;
  }

  if (is_queue_empty(q)) 
  {
    return NULL;
  }

  elem = q->elem[q->front];
  q->elem[q->front] = NULL;
  q->cnt--;

  if (q->front == q->rear) 
  {
    return elem;
  }

  if (q->front == QUEUE_SIZE - 1) 
  {
    q->front = 0;
  } else 
  {
    q->front++;
  }

  return elem;
}