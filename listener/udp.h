#ifndef _UDP_H
#define _UDP_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>

#define MAX_PKT_BUF_SIZE 1024

// 1번 인자: 수신 메시지.
// 2번 인자: 수신 메시지 크기.
// 3번 인자: 송신자 IP 주소.
// 4번 인자: 송신자 포트 번호.
typedef void (*recv_fn)(char *, uint32_t, char *, uint32_t);

pthread_t *
init_udp_server(char *addr, uint32_t port, recv_fn fn);

int
send_msg(char *addr, uint32_t port, char *msg, uint32_t msg_len);

char *
convert_n_to_p(uint32_t addr, char *buf);

#endif