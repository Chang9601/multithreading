#include <stdio.h>
#include "udp.h"

#define HOST "localhost"
#define PORT_1 3000
#define PORT_2 4000

void 
recv_pkt(char *pkt, uint32_t pkt_size, char *addr, uint32_t port) {
  printf("%s: 주소 = %s, 패킷 = %s, 크기 = %u\n", __func__, addr, pkt, pkt_size);
}

pthread_t *listener1, *listener2;

int
main(int argc, char *argv[]) {
  listener1 = listener2 = NULL;

  printf("UDP 포트 %d에서 대기 중.\n", PORT_1);
  listener1 = init_udp_server(HOST, PORT_1, recv_pkt);
  
  printf("UDP 포트 %d에서 대기 중.\n", PORT_2);
  listener2 = init_udp_server(HOST, PORT_2, recv_pkt);

  pthread_exit(NULL);

  exit(EXIT_SUCCESS);
}