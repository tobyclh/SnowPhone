#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#include <sys/socket.h>
#include <arpa/inet.h>

int main() {
  int s = socket(PF_INET, SOCK_STREAM, 0);
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;

  if (inet_aton("127.0.0.1", &addr.sin_addr) == 0) {
    perror("inet_aton");
    exit(1);
  }
  addr.sin_port = htons(50000);
  int ret = connect(s, (struct sockaddr *)&addr, sizeof(addr));
  int should_stop = 0;

  int buffer_size = 100000;
  unsigned char *buffer;
  buffer = (unsigned char *)calloc(buffer_size, sizeof(unsigned char));

  while (!should_stop) {
    int recv_count = read(s, buffer, buffer_size);
    printf("%c", *buffer);
  }
  close(s);
}
