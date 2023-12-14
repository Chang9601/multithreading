#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int N;

void *
thr_fn(void *arg) {
  pthread_t next_tid;

  int id = *(int *)arg;
  free(arg);

  if (id < N) {
    int *next_id = calloc(1, sizeof(*next_id));
    *next_id = id + 1;

    pthread_create(&next_tid, NULL, thr_fn, (void *)next_id);
    pthread_join(next_tid, NULL);
  }

  printf("%d\n", id);
}

void
create_thr(pthread_t *tid, int id) {
  int *_id = calloc(1, sizeof(*_id));
  *_id = id;

  pthread_create(tid, NULL, thr_fn, (void *)_id);
}

int
main(int argc, char *argv[]) {
  scanf("%d", &N);

  pthread_t tid;
  int id = 1;

  create_thr(&tid, id);
  pthread_join(tid, NULL);

  exit(EXIT_SUCCESS);
}