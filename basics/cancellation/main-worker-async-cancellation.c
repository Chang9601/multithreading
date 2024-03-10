#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#define N_THREADS 5
#define BUF_SIZE 64
#define FILE_SIZE 64

void *
write_to_file(void *arg)
{
  char file[FILE_SIZE], buf[BUF_SIZE];
  int len, cnt, *tid;
  FILE *fp;

  // pthread_attr_t 구조체에 포함되지 않은 두 가지 스레드 속성은 취소 가능 상태와 취소 유형이다. 이러한 속성은 pthread_cancel() 함수 호출에 응답하여 스레드의 동작에 영향을 미친다.
  // 취소 가능 상태 속성은 PTHREAD_CANCEL_ENABLE 또는 PTHREAD_CANCEL_DISABLE 중 하나이며 스레드는 pthread_setcancelstate() 함수를 호출하여 취소 가능 상태를 변경할 수 있다.
  // pthread_setcancelstate() 함수는 현재의 취소 가능 상태를 1번 인자로 설정하고 이전의 취소 가능 상태를 2번 인자가 가리키는 메모리 위치에 저장하는 단일 원자적인 작업을 수행한다.
  // 기본적으로 취소 요청이 발생하면 스레드는 취소 지점(cancellation point)에 도달할 때까지 계속 실행한다. 취소 지점은 스레드가 자신이 취소되었는지 확인하고 그렇다면 요청에 대응하는 지점이다.
  // 스레드는 PTHREAD_CANCEL_ENABLE를 기본 취소 가능 상태로 시작한다. 
  // 1번 인자가 PTHREAD_CANCEL_DISABLE로 설정되면 pthread_cancel() 함수 호출이 스레드를 종료하지 않는다. 대신 취소 요청은 스레드에 대해 대기 상태로 유지되고 취소 가능 상태가 다시 활성화되면 스레드는 다음 취소 지점에서 대기 중인 취소 요청에 반응한다.
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

  // 기본 취소 유형은 지연 취소(deferred cancellation)로 알려져 있다. 
  // pthread_cancel() 함수 호출 후에 실제 취소는 스레드가 취소 지점에 도달할 때 발생한다. 취소 유형은 pthread_setcanceltype() 함수를 호출하여 변경할 수 있다.
  // pthread_setcanceltype() 함수는 취소 유형을 1번 인자(PTHREAD_CANCEL_DEFERRED 또는 PTHREAD_CANCEL_ASYNCHRONOUS)으로 설정하고 2번 인자이 가리키는 정수에 이전 취소 유형을 반환한다.
  // 비동기 취소(asynchronous cancellation)는 지연 취소와 달리 언제든지 스레드가 취소될 수 있으며 스레드가 반드시 취소 지점에 도달할 필요가 없다.
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

  cnt = 0;
  tid = (int *)arg;

  sprintf(file, "thread_%d.txt", *tid);

  if ((fp = fopen(file, "w")) == NULL)
  {
    fprintf(stdout, "오류 발생! 파일 %s 열기 실패, errno = %d\n", file, errno);
    
    return (void *)-1;
  }

  while (1)
  {
    len = sprintf(buf, "%d. 스레드 %d\n", cnt++, *tid);
    fwrite(buf, sizeof(char), len, fp);
    fflush(fp);
    sleep(1);
  }

  return NULL;
}

int
main(int argc, char *argv[])
{
  pthread_t tids[N_THREADS];
  int i, id, ch, err, *tid;

  // N_THREADS 개수의 스레드를 생성한다.
  // 스레드 아이디를 인자로 전달한다.
  for (i = 0; i < N_THREADS; i++)
  {
    tid = calloc(1, sizeof(*tid));
    *tid = i;
    
    err = pthread_create(&tids[i], NULL, write_to_file, (void *)tid);
    if (err != 0)
    {
      printf("오류 발생! 스레드 생성 실패, errno = %d\n", err);
      exit(EXIT_FAILURE);      
    }
  }

  while (1)
  {
    printf("1. 스레드를 취소한다.\n");
    scanf("%d", &ch);

    printf("스레드 번호를 입력하시오. [0-%d]: ", N_THREADS - 1);
    scanf("%d", &id);

    if (id < 0 || id >= N_THREADS)
    {
      printf("유효하지 않은 스레드 번호!\n");
      continue;
    }

    switch (ch)
    {
      case 1:
        // 동일한 프로세스 내의 한 스레드는 pthread_cancel() 함수를 호출하여 다른 스레드의 취소를 요청할 수 있다.
        // 기본 상황에서 pthread_cancel() 함수는 인자로 주어진 스레드가 PTHREAD_CANCELED 인자를 사용하여 pthread_exit() 함수를 호출한 것처럼 동작하게 한다.
        // 그러나 스레드는 취소를 무시하거나 취소되는 방식을 제어하기로 선택할 수 있다. 
        // 참고로, pthread_cancel() 함수는 스레드가 종료될 때까지 기다리지 않으며 단순히 요청만 한다.
        // 스레드가 취소 가능하지(cancellable) 않으면 작동하지 않는다.
        pthread_cancel(tids[id]);
        break;
      default:
        continue;
    }
  }

  exit(EXIT_SUCCESS);
}