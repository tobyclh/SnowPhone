#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "too few arguments, 1 required.");
    exit(1);
  }
  char *server_port_number = argv[1];
  //int server_port_number = atoi(argv[1]);

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
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(atoi(server_port_number));

  if (bind(ss, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    perror("bind ss");
    exit(1);
  }
  if (listen(ss, 10) == -1) {
    perror("listen ss");
    exit(1);
  }

  struct sockaddr_in client_addr;
  socklen_t len = sizeof(struct sockaddr_in);
  int s = accept(ss, (struct sockaddr *)&client_addr, &len);
  fprintf(stderr, "OK2");
  if (s == -1) {
    perror("accept ss");
    exit(1);
  }

  read(s, data, N);
  //recv(s, data, N, 0);
  printf("data : %s\n", data);

  close(s);
  close(ss);

  return 0;
}
