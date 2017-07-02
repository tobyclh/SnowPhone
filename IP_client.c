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
  if (argc < 3) {
    fprintf(stderr, "too few arguments, 2 required.");
    exit(1);
  }
  char *server_ip_address = argv[1];
  if (strncmp(server_ip_address, "localhost", 9) == 0) {
    server_ip_address = "127.0.0.1";
  }
  char *server_port_number = argv[2];

  int N = 10000;
  unsigned char data[N];
  memset(data, 0, sizeof(data));

  int s = socket(PF_INET, SOCK_STREAM, 0);
  if (s == -1) {
    perror("socket s");
    exit(1);
  }

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;

  if (inet_aton(server_ip_address, &addr.sin_addr) == 0) {
    perror("inet_aton");
    exit(1);
  }

  addr.sin_port = htons(atoi(server_port_number));

  if (connect(s, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    perror("connect s");
    exit(1);
  }

  scanf("%s", data);
  write(s, data, N);
  //send(s, data, N, 0);

  close(s);

  return 0;
}
