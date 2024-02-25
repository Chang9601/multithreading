#include "queue.h"

struct queue *
init_queue(void) 
{
  struct queue *q;
  
  q = calloc(1, sizeof(*q));
  q->front = q->rear = q->cnt = 0;

  pthread_mutex_init(&q->mutex, NULL);
  pthread_cond_init(&q->cv, NULL);

  return q;
}

void
destroy_queue(struct queue *q)
{
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