#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#include <sys/socket.h>
#include <arpa/inet.h>

char *rec_command = "rec -t raw -b 16 -c 2 -e s -r 44100 -";
char *play_command = "play -t raw -b 16 -c 2 -e s -r 44100 -";

FILE *rec_fp, *play_fp;

int N = 1500;

int s;

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

  unsigned char send_data[N];
  unsigned char recv_data[N];
  unsigned char buf[N];

  memset(send_data, 0, sizeof(send_data));
  memset(recv_data, 0, sizeof(recv_data));

  s = socket(PF_INET, SOCK_STREAM, 0);
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

  if ((rec_fp = popen(rec_command, "r")) == NULL) {
    fprintf(stderr, "recコマンドのオープンに失敗しました.\n");
    exit(EXIT_FAILURE);
  }
  if ((play_fp = popen(play_command, "w")) == NULL) {
    fprintf(stderr, "playコマンドのオープンに失敗しました.\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    /*
    scanf("%s", send_data);
    if (strncmp(send_data, "finish", 6) == 0) {
      break;
    }
    */

    fread(buf, 1, N, rec_fp);
    fprintf(stderr, "fread complete\n");

    write(s, buf, N);
    fprintf(stderr, "write complete\n");
    /*
    read(s, buf, N);
    fprintf(stderr, "read complete\n");

    fwrite(buf, 1, N, play_fp);
    fprintf(stderr, "fwrite complete\n");
*/

    memset(send_data, 0, sizeof(send_data));
    memset(recv_data, 0, sizeof(recv_data));
    memset(buf, 0, sizeof(recv_data));
  }

  close(s);

  return 0;
}
