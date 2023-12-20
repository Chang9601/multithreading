#include "udp.h"

typedef struct _thr_pkt {
  char addr[16];
  uint32_t port;
  recv_fn fn;
} thr_pkt;

static void *
_init_udp_serv(void *arg) {
  thr_pkt *pkt = (thr_pkt *)arg;

  char addr[16];
  strncpy(addr, pkt->addr, 16);
  uint32_t port = pkt->port;
  recv_fn fn = pkt->fn;

  free(pkt);
  pkt = NULL;

  // AF_INET: IPv4 프로토콜
  // SOCK_DGRAM: 데이터그램 소켓
  // IPPROTO_UDP: UDP 전송 프로토콜
  int sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  if (sock_fd < 0) {
    printf("소켓 생성 실패!");
    exit(EXIT_FAILURE);
  }

  // socketaddr_in: IPv4 소켓 주소 구조체
  // in_addr: 네트워크 바이트 순서로 32비트 IPv4 주소를 가지는 구조체
  // INADDR_ANY:  여러 개의 인터페이스를 가진 경우 서버가 어떤 인터페이스에서든 클라이언트 연결을 수락할 수 있다. 즉, 서버가 모든 가능한 네트워크 인터페이스에서 연결을 수락하고 대기한다.
  struct sockaddr_in serv;
  serv.sin_family = AF_INET;
  serv.sin_port = port;
  serv.sin_addr.s_addr = INADDR_ANY;

  if (bind(sock_fd, (struct sockaddr *)&serv, sizeof(serv)) < 0) {
    printf("소켓 바인딩 실패!");
    exit(EXIT_FAILURE);
  }

  char *buf = calloc(1, MAX_PACKET_BUF_SIZE);

  struct sockaddr_in cli;
  int bytes = 0;
  socklen_t addr_len = sizeof(cli);

  while (1) {
    memset(buf, 0, MAX_PACKET_BUF_SIZE);

    // recvfrom() 함수는 블록킹 API. 즉, UDP 소켓으로 데이터가 도착할 때까지 수신자 스레드는 블록된다.
    // 따라서, fn() 함수는 수신자 스레드가 패킷을 수신하고 그 패킷이 버퍼에 사용 가능한 상태일 때에만 실행된다.
    // 데이터그램 프로토콜과 함께 사용되면 반환 값은 받은 데이터그램에 있는 사용자 데이터의 양이다.
    // 반환된 소켓 주소 구조체(cli)의 내용은 데이터그램을 보낸 사람(UDP) 또는 연결을 시작한 사람(TCP)
    bytes = recvfrom(sock_fd, buf, MAX_PACKET_BUF_SIZE, 0, (struct sockaddr *)&cli, &addr_len);

    // 호스트 바이트 순서로 된 IP 주소를 네트워크 바이트 순서로 변환한 후 문자열 형식으로 변환한다.
    fn(buf, bytes, translate_n_to_p(htonl(cli.sin_addr.s_addr), 0), cli.sin_port);
  }

  exit(EXIT_SUCCESS);
}

pthread_t *
init_udp_serv(char *addr, uint32_t port, recv_fn fn) {
  
  pthread_attr_t attr;
  pthread_t *thr;
  thr_pkt *pkt;

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  pkt = calloc(1, sizeof(*pkt));

  strncpy(pkt->addr, addr, 16);
  pkt->port = port;
  pkt->fn = fn;

  thr = calloc(1, sizeof(*thr));
  pthread_create(thr, &attr, _init_udp_serv, (void *)pkt);

  pthread_attr_destroy(&attr);

  return thr;
}

int
send_msg(char *addr, uint32_t port, char *msg, uint32_t msg_len) {
  struct sockaddr_in dest;

  dest.sin_family = AF_INET;
  dest.sin_port = port;
  struct hostent *host = (struct hostent *)gethostbyname(addr);
  // h_addr은 hostent 구조체의 필드로, h_addr_list의 첫 번째 IP 주소를 가진다.
  dest.sin_addr = *((struct in_addr *)host->h_addr);
  //int addr_len = sizeof(struct sockaddr);f
  int sock_fd;

  sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  if (sock_fd < 0) {
    printf("소켓 생성 실패!");
    exit(EXIT_FAILURE);
  }

  // 소켓 주소 구조체(dest)의 내용은 데이터그램을 보낼 위치의 프로토콜 주소(UDP) 또는 연결을 설정할 대상(TCP)
  int bytes = sendto(sock_fd, msg, msg_len, 0, (struct sockaddr *)&dest, sizeof(struct sockaddr));
  close(sock_fd);

  return bytes;
}

char *
translate_n_to_p(uint32_t addr, char *buf) {
  char *out = NULL;

  static char ip[16];
  out = buf ? buf : ip;
  memset(out, 0, 16);
  addr = htonl(addr);
  // p는 표현(presentation), n은 숫자(numeric)를 나타낸다.
  inet_ntop(AF_INET, &addr, out, 16);
  out[15] = '\0';

  return out;
}