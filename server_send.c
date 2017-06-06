#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sox.h>
#include <sys/socket.h>
#include <assert.h>
int setup_socket(int port)
{
  int ss = socket(PF_INET, SOCK_STREAM, 0);
  if (ss == -1)
  {
    perror("socket ss");
    exit(1);
  }

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;

  bind(ss, (struct sockaddr *)&addr, sizeof(addr));
  listen(ss, 10);
  return ss;
}



int main()
{
  // buffer size 48000Hz * 2ch * 5sec
  size_t n = 48000 * 2 * 5;
  sox_sample_t *buf = malloc(sizeof(sox_sample_t) * n);
  sox_init();
  sox_format_t *ft = sox_open_read("default", 0, 0, "pulseaudio");

  struct sockaddr_in client_addr;
  socklen_t len = sizeof(struct sockaddr_in);
  int port;
  scanf("%d", &port);
  int ss = setup_socket(port);
  int s = accept(ss, (struct sockaddr *)&client_addr, &len);
  while (1)
  {
    int m = sox_read(ft, buf, n);
    assert(m == n);

    write(s, buf, n);
  }
  sox_close(ft);
  sox_quit();
  return 0;
}
