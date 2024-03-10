#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include "queue.h"

int 
_prod_num()
{
	static int num = 0;
	num++;

	return num;
}

struct queue *q;

const char *prod1 = "생산자 스레드 1";
const char *prod2 = "생산자 스레드 2";
const char *cons1 = "소비자 스레드 1";
const char *cons2 = "소비자 스레드 2";

void *
prod_fn(void *arg)
{
	int num;
	char *thr_name;
	
	thr_name = (char *)arg;

	printf("%s: 뮤텍스를 잠그려고 대기 중이다.\n", thr_name);
	pthread_mutex_lock(&q->mutex);
	printf("%s: 뮤텍스를 잠근다.\n", thr_name);

	while (is_queue_full(q))
	{
		printf("%s: 큐가 가득 차 있어서 스스로를 블록한다.\n", thr_name);
		pthread_cond_wait(&q->cv, &q->mutex);
		printf("%s: 깨어나서 큐의 상태를 다시 한 번 확인한다.\n", thr_name);
	}

	// 큐가 비어있지 않으면 강제로 프로그램을 종료시킨다.
	assert(is_queue_empty(q));

	while (!is_queue_full(q))
	{
		num = _prod_num();
		printf("%s: 정수 %d 생산한다.\n", thr_name, num);

		enqueue(q, (void *)(intptr_t)num);
		printf("%s: 정수 %d 큐에 삽입한다. 큐의 크기 = %d\n", thr_name, num, q->cnt);
	}

	printf("%s: 큐가 가득 찼기 때문에 신호를 보내고 뮤텍스를 푼다.\n", thr_name);
	pthread_cond_broadcast(&q->cv);
	// pthread_cond_signal(&q->cv);

	pthread_mutex_unlock(&q->mutex);
	printf("%s: 작업을 완료하고 종료한다.\n", thr_name);

	return NULL;
}

void *
cons_fn(void *arg) 
{
	int num;
	char *thr_name;
	
	thr_name = (char *)arg;
	
	// 뮤텍스로 상호배제를 강제한다.
	printf("%s: 뮤텍스를 잠그려고 대기 중이다.\n", thr_name);
	pthread_mutex_lock(&q->mutex);
	printf("%s: 뮤텍스를 잠근다.\n", thr_name);

	// 의도하지 않은 기상(spurious wake-up)은 스레드가 조건이 충족되지 않아서 블록되었는데 신호를 받고 실행을 시작하면 블록된 이유인 충족되지 않은 조건이 여전히 남아있다.
	// 스레드는 깨어나서 실행을 재개하기 전에 조건문(predicate condition)을 반드시 다시 확인해야 한다.
	// 왜? 스레드가 신호를 받고 깨어나는 동안 프로세스의 다른 스레드가 자원 상태를 변경할 수 있으므로, 이전 스레드는 블록된 상태로 유지되어야 한다. 
	// 따라서 스레드가 깨어나면, 자원에 접근하기 위한 조건이 충족될 때만 스레드가 자원을 처리하도록 보장하기 위해 다시 조문건을 확인해야 한다.
	// if 문이 아니라 while 문으로 이를 구현한다.
	while (is_queue_empty(q))
	{
		printf("%s: 큐가 비어 있어서 스스로를 블록한다.\n", thr_name);

		// pthread_cond_wait() 함수에 전달되는 뮤텍스는 조건을 보호한다. 
		// 호출자는 함수에 잠겨진 뮤텍스를 전달하며 함수는 호출 스레드를 조건이 변경될 때까지 대기하는 스레드 목록에 원자적으로 넣고 뮤텍스를 잠금 해제한다.
		// 이로써 조건을 확인하는 시간과 스레드가 조건이 변경되기를 기다리는 시간 사이의 창문을 닫아서 스레드가 조건이 변경되는 것을 놓치지 않는다.
		// pthread_cond_wait() 함수가 반환되면 뮤텍스가 다시 잠겨진다.

		// pthread_cond_wait() 함수 호출 시.
		// 1. 스레드가 블록된다(조건 변수의 역할.).
		// 2. 뮤텍스 소유권이 호출한 스레드로부터 빼앗겨서 사용 가능하게 선언된다.
		// 다른 스레드가 pthread_cond_signal() 함수를 호출하여 블록된 스레드가 신호를 받을 시.
	  // 1. 스레드가 실행 준비(ready-to-execute) 상태로 전환되어 뮤텍스가 풀릴 때까지 대기한다.
		// 2. 신호를 보낸 스레드에 의해 뮤텍스가 해제되면 해당 뮤텍스에 대한 잠금을 얻는다.
		// 3. 스레드는 실행을 재개한다.
		pthread_cond_wait(&q->cv, &q->mutex);

		printf("%s: 깨어나서 큐의 상태를 다시 한 번 확인한다.\n", thr_name);
	}

	// 큐가 가득차지 않으면 강제로 프로그램을 종료시킨다.
	assert(is_queue_full(q));

	while (!is_queue_empty(q))
	{
		num = (int)(intptr_t)dequeue(q);
		printf("%s: 정수 %d 소모한다. 큐의 크기 = %d\n", thr_name, num, q->cnt);
	}

	printf("%s: 큐가 비었기 때문에 블록되어 있는 소비자 스레드에 신호를 보낸다.\n", thr_name);
	// 조건이 충족되었음을 스레드에 알리기 위해 두 가지 함수가 있다.
	// pthread_cond_signal() 함수는 조건에서 대기 중인 최소한 하나의 스레드를 깨우고 pthread_cond_broadcast() 함수는 조건에서 대기 중인 모든 스레드를 깨운다.
	// pthread_cond_signal() 함수 또는 pthread_cond_broadcast() 함수를 호출할 때 스레드나 조건에 신호를 보낸다고 말한다. 
	// 조건의 상태를 변경한 후에만 스레드에게 신호를 보내야 하는 주의가 필요하다.

	// pthread_cond_broadcast() 함수는 동일한 조건 변수에서 블록된 여러 스레드에게 신호를 보내기 위해 사용된다.
	// 각각의 스레드는 운영체제의 스케줄링 정책 순서대로 신호를 받고 블록이 해제되어 자신의 임계 영역에서 나와 실행을 재개한다. 
	// 단, 단 하나의 스레드만이 임계 영역에서 나오고 한 번에 하나뿐이다.
	// 임계 영역은 한 번에 하나의 스레드에 의해 실행되는 코드 조각이기 때문에 상호 배제의 원칙이 절대로 위배되지 않는다.
	// 만약 이 함수를 사용하지 않고 모든 블록된 스레드가 실행을 재개하길 원한다면 스레드의 개수만큼 pthread_cond_signal() 함수를 호출해야 한다.
	pthread_cond_broadcast(&q->cv);

	// pthread_cond_signal() 함수에 의해 발생하는 교착상태 발생 조건.
	// 1. 생산자 스레드에서 생성된 신호가 다른 생성자 스레드에서만 처리되는 경우.
	// 2. 소비자 스레드에서 생성된 신호가 다른 소비자 스레드에서만 처리되는 경우.
	// pthread_cond_broadcast()를 사용하면 블록된 모든 스레드가 하나씩 신호를 받는다. 
	// 즉, 생산자 스레드는 다른 블록된 소비자 스레드에게 신호를 보내고 소비자 스레드는 다른 블록된 생산자 스레드에게 신호를 보낸다.
	// 따라서, 교착상태 조건이 충족되지 않는다.
	// pthread_cond_signal(&q->cv);
	
	printf("%s: 뮤텍스를 푼다.\n", thr_name);
	pthread_mutex_unlock(&q->mutex);
	printf("%s: 작업을 완료하고 종료한다.\n", thr_name);

	return NULL;
}

int
main(int argc, char *argv[]) 
{
  pthread_attr_t attr;
	pthread_t prod_tid1, prod_tid2; 
	pthread_t cons_tid1, cons_tid2;
  //int err;

	q = init_queue();

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	pthread_create(&prod_tid1, &attr, prod_fn, (void *)prod1);
	pthread_create(&prod_tid2, &attr, prod_fn, (void *)prod2);
	pthread_create(&cons_tid1, &attr, cons_fn, (void *)cons1);
	pthread_create(&cons_tid2, &attr, cons_fn, (void *)cons2);

	pthread_join(prod_tid1, NULL);
	pthread_join(prod_tid2, NULL);
	pthread_join(cons_tid1, NULL);
	pthread_join(cons_tid2, NULL);

	pthread_attr_destroy(&attr)
	destroy_queue(q);

	printf("프로그램 종료.\n");
	pthread_exit(NULL);

	exit(EXIT_SUCCESS);
}