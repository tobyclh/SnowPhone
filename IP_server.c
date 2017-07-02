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

int finish_flag = 0;

/*
void get_stdin() {
  while (1) {
    scanf("%s", control);
    if (strncmp(control, "finish", 6)) {
      finish_flag = 1;
      return;
    }
  }
}
*/

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "too few arguments, 1 required.");
    exit(1);
  }
  char *server_port_number = argv[1];
  //int server_port_number = atoi(argv[1]);

  unsigned char control[N];
  unsigned char send_data[N];
  unsigned char recv_data[N];
  unsigned char buf[N];

  memset(control, 0, sizeof(control));
  memset(send_data, 0, sizeof(send_data));
  memset(recv_data, 0, sizeof(recv_data));
  memset(buf, 0, sizeof(recv_data));

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
  s = accept(ss, (struct sockaddr *)&client_addr, &len); // file descriptor
  if (s == -1) {
    perror("accept ss");
    exit(1);
  }
  fprintf(stdout, "client accepted.\n");

  if ((rec_fp = popen(rec_command, "r")) == NULL) {
    fprintf(stderr, "recコマンドのオープンに失敗しました.\n");
    exit(EXIT_FAILURE);
  }
  if ((play_fp = popen(play_command, "w")) == NULL) {
    fprintf(stderr, "playコマンドのオープンに失敗しました.\n");
    exit(EXIT_FAILURE);
  }

  fprintf(stderr, "popen done\n");

  while (1) {
    /*
    scanf("%s", send_data);
    if (strncmp(send_data, "finish", 6) == 0) {
      break;
    }
    */

/*
    fread(buf, 1, N, rec_fp);
    fprintf(stderr, "fread complete\n");

    write(s, buf, N);
    fprintf(stderr, "write complete\n");
    */

    read(s, buf, N);
    fprintf(stderr, "read complete\n");

    fwrite(buf, 1, N, play_fp);
    fprintf(stderr, "fwrite complete\n");

    //fwrite(s, rec_fp, N);
    //fread(s, play_fp, N);
    //send(s, rec_fp, N, 0);
    //recv(s, play_fp, N, 0);
    //printf("recv_data : %s\n", recv_data);

    memset(send_data, 0, sizeof(send_data));
    memset(recv_data, 0, sizeof(recv_data));
  }

  close(s);
  close(ss);

  return 0;
}
