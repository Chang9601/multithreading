#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> // 스레드 사용 시 필요한 헤더
#include <unistd.h>

static void *
thr_fn(void *args) {

  char *input = (char *)args;
  int cnt = 1;

  while (cnt < 20) {
    printf("입력 = %s\n", input);
    sleep(1);

  if (cnt == 10) {
    pthread_exit(0);
  }

    cnt++;
  }
}

void
create_thr() {
  pthread_t tid;
  
  static char *input = "스레드 #1";
  
  // 스레드 분기 지점
  // 4번째 인자는 반드시 힙 영역 혹은 데이터 영역 메모리
  int err = pthread_create(&tid, NULL, thr_fn, (void *)input);

  if (err != 0) {
    printf("오류 발생! 스레드 생성 실패, errno = %d\n", err);
    exit(EXIT_FAILURE);    
  }
}

int
main(int args, char *argv[]) {

  create_thr();
  printf("메인 스레드 중지\n");
  pthread_exit(0); // 메인 스레드가 종료되어도 자식 스레드는 실행 중이다.
  // pause(); pause() 함수가 주석으로 처리되면 메인 스레드 종료 시 모든 자식 스레드가 종료된다.

  exit(EXIT_SUCCESS);
}
