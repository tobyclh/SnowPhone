#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#include <sys/socket.h>

int main() {
  int N = 10000;
  unsigned char data[N];
  memset(data, 0, sizeof(data));

  int ss = socket(PF_INET, SOCK_STREAM, 0);
  if (ss == -1) {
    perror("socket ss");
    exit(1);
  }

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(50000);
  addr.sin_addr.s_addr = INADDR_ANY;

  bind(ss, (struct sockaddr *)&addr, sizeof(addr));
  listen(ss, 10);

  struct sockaddr_in client_addr;
  socklen_t len = sizeof(struct sockaddr_in);
  int s = accept(ss, (struct sockaddr *)&client_addr, &len);
  data[0] = 'h';
  write(s, data, N);

  return 0;
}

