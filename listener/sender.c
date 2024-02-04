#include "udp.h"

int
main(int argc, char *argv[]) {
  printf("대상 = [%s, %d]\n", argv[1], atoi(argv[2]));
  
  send_msg(argv[1], atoi(argv[2]), argv[3], strlen(argv[3]));

  exit(EXIT_SUCCESS);
}