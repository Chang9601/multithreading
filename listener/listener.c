#include <stdio.h>
#include "udp.h"

void 
recv_pkt(char *pkt, uint32_t pkt_size, char *ip_addr, uint32_t port) {
  printf("recv_pkt(): 주소 = %s, 패킷 = %s, 크기 = %u\n", ip_addr, pkt, pkt_size);
}

pthread_t *listener1 = NULL;
pthread_t *listener2 = NULL;

int
main(int argc, char *argv[]) {
  printf("UDP 포트 3000에서 대기 중\n");
  listener1 = init_udp_serv("127.0.0.1", 3000, recv_pkt);
  
  printf("UDP 포트 4000에서 대기 중\n");
  listener2 = init_udp_serv("127.0.0.1", 4000, recv_pkt);

  // 메인 스레드가 종료되어도 자식 스레드는 실행 중이다.
  pthread_exit((void *)0);

  exit(EXIT_SUCCESS);
}