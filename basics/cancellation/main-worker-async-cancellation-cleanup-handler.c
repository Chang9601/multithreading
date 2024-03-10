#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#define N_THREADS 5
#define BUF_SIZE 64
#define FILE_SIZE 64

void
mem_cleanup(void *arg)
{
  printf("%s...\n", __func__);

  free(arg);
}

void
file_cleanup(void *arg)
{
  printf("%s...\n", __func__);
  
  fclose((FILE *)arg);
}

void *
write_to_file(void *arg)
{
  char file[FILE_SIZE], buf[BUF_SIZE];
  int len, cnt, *tid;
  FILE *fp;

  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

  cnt = 0;
  // 힙 메모리에 존재하는 arg, 스레드가 취소 신호를 받으면 메모리 해제가 필요하다.
  tid = (int *)arg;

  // 스레드가 취소 신호를 받으면 정리 함수는 포인터 주소의 메모리를 해제한다.
  pthread_cleanup_push(mem_cleanup, arg);

  sprintf(file, "thread_%d.txt", *tid);

  if ((fp = fopen(file, "w")) == NULL)
  {
    fprintf(stdout, "오류 발생! 파일 %s 열기 실패, errno = %d\n", file, errno);
    // 스레드가 시작 루틴에서 반환하여 종료되면 정리 핸들러가 호출되지 않지만, 이 동작은 구현에 따라 다를 수 있다.
    // 만약 파일을 열 수 없으면 arg 변수를 해제해야 하는데 스레드가 반환문(return (void *)1;)을 통해 종료될 때 스레드 정리 스택에 있는 정리 핸들러는 호출되지 않는다.
    // 하지만, pthread_exit() 함수를 사용하여 스레드를 종료하면 정리 핸들러가 호출된다.
    pthread_exit((void *)-1);
  }

  // 열린 파일은 스레드가 취소될 때 닫혀져야 한다.
  // 즉, 파일이 성공적으로 열리면 스레드는 파일을 닫는 정리 핸들러가 필요하다.
  pthread_cleanup_push(file_cleanup, fp);
  
  // 스레드가 취소 신호를 받았을 때 while 문 안의 코드를 실행 중이다.
  while (1)
  {
    len = sprintf(buf, "%d. 스레드 %d\n", cnt++, *tid);
    fwrite(buf, sizeof(char), len, fp);
    fflush(fp);
    sleep(1);
  }
  
  // 스레드는 자신이 종료될 때 호출될 함수를 배열할 수 있다. 이러한 함수를 스레드 정리 핸들러(thread cleanup handlers)라고 부른다.
  // 스레드에 대해 하나 이상의 정리 핸들러를 설정할 수 있다. 핸들러들은 스택에 기록되어 등록된 역순으로 실행되기 때문에 등록된 순서와 반대 순서로 실행된다.
  // pthread_cleanup_push() 함수는 1번 인자인 정리 핸들러를 예약하여 스레드가 다음 중 하나의 작업을 수행할 때 단일 인자인 2번 인자와 함께 호출되도록 한다.
  // 1. pthread_exit() 함수를 호출한다.
  // 2. 취소 요청에 응답한다.
  // 3. pthread_cleanup_pop() 함수를 1번 인자가 0이 아닌 값을 가지고 호출한다.
  // 만약 pthread_cleanup_pop() 함수의 1번 인자가 0으로 설정되면 정리 핸들러는 호출되지 않는다. 
  // 어느 경우에도 pthread_cleanup_pop() 함수는 pthread_cleanup_push() 함수의 마지막 호출에 의해 설정된 정리 핸들러를 제거한다.
  // 이 두 함수들과 관련된 제약 중 하나는 이들이 매크로로 구현될 수 있기 때문에 스레드 내에서 동일한 범위에서 일치하는 쌍으로 사용되어야 한다. 
  // 즉, pthread_cleanup_push() 함수의 매크로 정의에는 { 문자가 포함될 수 있는데, 이 경우 일치하는 } 문자는 pthread_cleanup_pop() 함수의 정의에 있다.

  // 정리 핸들러를 정리 스택에 넣었기에 스레드가 성공적으로 종료되면 정리 핸들러를 정리 스택에서 제거하기 위해 호출해야 한다.
  // 두 번 호출하는 이유는 두 개의 정리 핸들러를 정리 스택에 넣었기 때문이다.
  // 인자로 0을 전달하면 정리 스택의 맨 위에서 정리 핸들러를 제거한다.
  // 인자로 0 이외의 값을 전달하면 정리 스택의 맨 위에서 정리 핸들러를 제거하고 호출한다.
  pthread_cleanup_pop(1);
  pthread_cleanup_pop(1);

  pthread_exit(NULL);
}

int
main(int argc, char *argv[])
{
  pthread_t tids[N_THREADS];
  int i, id, ch, err, *tid;

  for (i = 0; i< N_THREADS; i++)
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
        pthread_cancel(tids[id]);
        break;
      default:
        continue;
    }
  }

  exit(EXIT_SUCCESS);
}