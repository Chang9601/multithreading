#ifndef _DP_H
#define _DP_H

#include <stdbool.h>
#include <pthread.h>

// 철학자는 스레드.
struct phil {
  int id;
  pthread_t tid;
  int cnt; // 케이크를 먹은 횟수
};

// 숟가락은 공유 자원.
struct sp {
  int id;
  bool is_taken; // 숟가락 사용 여부.
  struct phil *phil; // 사용하는 철학자.
  pthread_mutex_t mutex; // 상호 배제
  pthread_cond_t cv; // 조율
};

struct sp *
get_right_sp(struct phil *phil);

struct sp *
get_left_sp(struct phil *phil);

void
eat(struct phil *phil);

bool
hold_both_sps(struct phil *phil);

void
release_both_sps(struct phil *phil);

#endif