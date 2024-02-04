#include "udp.h"

struct thr_pkt {
  char addr[16];
  uint32_t port;
  recv_fn fn;
};

void
mem_cleanup(void *arg) {
  printf("%s...\n", __func__);

  free(arg);

  return;
}

void
sock_cleanup(void *arg) {
  printf("%s...\n", __func__);

  int sock_fd = *(int *)arg;

  close(sock_fd);

  return;
}

static void *
_init_udp_server(void *arg) {
  struct thr_pkt *pkt; 
  char addr[16], *buf;
  uint32_t port;
  recv_fn fn;
  int sock_fd, bytes;
  struct sockaddr_in server, client;
  socklen_t addr_len;

  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

  pkt = (struct thr_pkt *)arg;
  strncpy(addr, pkt->addr, 16);
  port = pkt->port;
  fn = pkt->fn;

  free(pkt);
  pkt = NULL;

  // AF_INET: IPv4 프로토콜.
  // SOCK_DGRAM: 데이터그램 소켓.
  // IPPROTO_UDP: UDP 전송 프로토콜.
  sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  if (sock_fd < 0) {
    printf("오류 발생! 소켓 생성 실패, errno = %d\n", errno);
    
    return (void *)-1;
  }

  pthread_cleanup_push(sock_cleanup, (void *)(intptr_t)sock_fd);

  // socketaddr_in: IPv4 소켓 주소 구조체.
  // in_addr: 네트워크 바이트 순서로 32비트 IPv4 주소를 가지는 구조체.
  // INADDR_ANY: 여러 개의 인터페이스를 가진 경우 서버가 어떤 인터페이스에서든 클라이언트 연결을 수락할 수 있다. 즉, 서버가 모든 가능한 네트워크 인터페이스에서 연결을 수락하고 대기한다.
  server.sin_family = AF_INET;
  server.sin_port = port;
  server.sin_addr.s_addr = INADDR_ANY;

  if (bind(sock_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
    printf("오류 발생! 소켓 바인딩 실패, errno = %d\n", errno);
    
    // return (void *)-2;
    pthread_exit((void *)-2);
  }

  buf = calloc(1, MAX_PKT_BUF_SIZE);
  bytes = 0;
  addr_len = sizeof(client);

  pthread_cleanup_push(mem_cleanup, (void *)buf);

  while (1) {
    memset(buf, 0, MAX_PKT_BUF_SIZE);

    // recvfrom() 함수는 블록킹 API. 즉, UDP 소켓으로 데이터가 도착할 때까지 수신자 스레드는 블록된다.
    // 따라서, fn() 함수는 수신자 스레드가 패킷을 수신하고 그 패킷이 버퍼에 사용 가능한 상태일 때에만 실행된다.
    // 데이터그램 프로토콜과 함께 사용되면 반환 값은 받은 데이터그램에 있는 사용자 데이터의 양이다.
    // 반환된 소켓 주소 구조체(client)의 내용은 데이터그램을 보낸 사람(UDP) 또는 연결을 시작한 사람(TCP)
    bytes = recvfrom(sock_fd, buf, MAX_PKT_BUF_SIZE, 0, (struct sockaddr *)&client, &addr_len);

    // 호스트 바이트 순서로 된 IP 주소를 네트워크 바이트 순서로 변환한 후 문자열 형식으로 변환한다.
    fn(buf, bytes, convert_n_to_p(htonl(client.sin_addr.s_addr), 0), client.sin_port);

    // 블록킹 전에 취소해야 하는지 확인하기 위해 취소 지점을 삽입한다.
    pthread_testcancel();
  }

  pthread_cleanup_pop(1);
  pthread_cleanup_pop(1);

  // return NULL;
  pthread_exit(NULL);
}

pthread_t *
init_udp_server(char *addr, uint32_t port, recv_fn fn) {
  pthread_attr_t attr;
  pthread_t *tid;
  struct thr_pkt *pkt;

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  pkt = calloc(1, sizeof(*pkt));

  strncpy(pkt->addr, addr, 16);
  pkt->port = port;
  pkt->fn = fn;

  tid = calloc(1, sizeof(*tid));
  pthread_create(tid, &attr, _init_udp_server, (void *)pkt);

  pthread_attr_destroy(&attr);

  return tid;
}

int
send_msg(char *addr, uint32_t port, char *msg, uint32_t msg_len) {
  struct sockaddr_in dest;
  struct hostent *host;
  int sock_fd, bytes;

  dest.sin_family = AF_INET;
  dest.sin_port = port;

  host = (struct hostent *)gethostbyname(addr);
  // h_addr은 hostent 구조체의 필드로, h_addr_list의 첫 번째 IP 주소를 가진다.
  dest.sin_addr = *((struct in_addr *)host->h_addr);
  //int addr_len = sizeof(struct sockaddr);

  sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  if (sock_fd < 0) {
    printf("오류 발생! 소켓 생성 실패, errno = %d\n", errno);

    return -1;
  }

  // 소켓 주소 구조체(dest)의 내용은 데이터그램을 보낼 위치의 프로토콜 주소(UDP) 또는 연결을 설정할 대상(TCP)
  bytes = sendto(sock_fd, msg, msg_len, 0, (struct sockaddr *)&dest, sizeof(struct sockaddr));
  close(sock_fd);

  return bytes;
}

char *
convert_n_to_p(uint32_t addr, char *buf) {
  static char ip[16];
  char *out;

  out = buf ? buf : ip;

  memset(out, 0, 16);
  
  addr = htonl(addr);
  // p는 표현(presentation), n은 숫자(numeric)를 나타낸다.
  inet_ntop(AF_INET, &addr, out, 16);
  
  out[15] = '\0';

  return out;
}