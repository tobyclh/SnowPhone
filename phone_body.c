#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include <pthread.h>

char *rec_command = "rec -t raw -b 16 -c 2 -e s -r 44100 - bandpass 1850 3100";
char *play_command = "play -t raw -b 16 -c 2 -e s -r 44100 - bandpass 1850 3100";

FILE *rec_fp, *play_fp;
FILE *video_stream_fp;

int N = 1024; // 適切な値を設定
int s;
struct sockaddr_in addr;

unsigned char *rec_data;
unsigned char *play_data;

char *video_stream_command = "";

void *rec_send(void *arg) {
  while (1) {
    fread(rec_data, 1, N, rec_fp); // read from rec_fp to rec_data
    // read(rec_fp, rec_data, N); // read from rec_fp to rec_data
    // fprintf(stderr, "fread complete\n");

    write(s, rec_data, N); // write to s from rec_data
    // fprintf(stderr, "write complete\n");
  }

  return arg;
}

void *recv_play(void *arg) {
  while (1) {
    read(s, play_data, N); // read from s to play_data
    // fprintf(stderr, "read complete\n");

    fwrite(play_data, 1, N, play_fp); // write to play_fp from play_data
    // write(play_fp, play_data, N); // write to play_fp from play_data
    // fprintf(stderr, "fwrite complete\n");
  }

  return arg;
}

void *get_stdin(void *arg) {
  int m = 1000;
  char *control = calloc(m, sizeof(char *));
  while (1) {
    scanf("%s", control);
    if (strncmp(control, "finish", 6) == 0) {
      return arg;
    }
    memset(control, 0, m);
  }

  return arg;
}

int main(int argc, char **argv) {
  int is_server = 0;
  if (argc < 3) {
    fprintf(stderr, "too few arguments, 2 required.");
    exit(1);
  }
  char *server_ip_address = argv[1];
  if (strncmp(server_ip_address, "s", 1) == 0) {
    is_server = 1;
  } else if (strncmp(server_ip_address, "localhost", 9) == 0) {
    server_ip_address = "127.0.0.1";
  }
  char *server_port_number = argv[2];

  rec_data = calloc(N, sizeof(unsigned char));
  play_data = calloc(N, sizeof(unsigned char));

  int ss;
  if (is_server) {
    ss = socket(PF_INET, SOCK_STREAM, 0); // file descriptor
    if (ss == -1) {
      perror("socket ss");
      exit(1);
    }

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
    s = accept(ss, (struct sockaddr *)&client_addr, &len);
    if (s == -1) {
      perror("accept ss");
      exit(1);
    }
    fprintf(stdout, "client accepted.\n");
  } else {
    s = socket(PF_INET, SOCK_STREAM, 0);
    if (s == -1) {
      perror("socket s");
      exit(1);
    }

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
  }
  fprintf(stderr, "OK\n");
  video_stream_command = calloc(100, sizeof(unsigned char *));

  if (is_server) {
    sprintf(video_stream_command, "./server %d", atoi(server_port_number) + 1);
  } else {
    sprintf(video_stream_command, "./client %s %d", server_ip_address, atoi(server_port_number) + 1);
  }
  fprintf(stderr, "%s\n", video_stream_command);

  if ((rec_fp = popen(rec_command, "r")) == NULL) {
    fprintf(stderr, "recコマンドのオープンに失敗しました.\n");
    exit(EXIT_FAILURE);
  }
  if ((play_fp = popen(play_command, "w")) == NULL) {
    fprintf(stderr, "playコマンドのオープンに失敗しました.\n");
    exit(EXIT_FAILURE);
  }

  if ((video_stream_fp = popen(video_stream_command, "r")) == NULL) {
    fprintf(stderr, "video streamコマンドのオープンに失敗しました.\n");
    exit(EXIT_FAILURE);
  }

  pid_t p_pid;
  pthread_t rec_thread_id, play_thread_id, control_thread_id;
  int status;
  void *result;

  p_pid = getpid();

  status = pthread_create(&rec_thread_id, NULL, rec_send, (void *)NULL);
  if (status != 0){
    fprintf(stderr, "pthread_create : %s", strerror(status));
  }
  else{
    printf("[%d]thread_id1 = %d\n", p_pid, rec_thread_id);
  }

  status = pthread_create(&play_thread_id, NULL, recv_play, (void *)NULL);
  if (status != 0){
    fprintf(stderr, "pthread_create : %s", strerror(status));
  }
  else{
    printf("[%d]thread_id1 = %d\n", p_pid, play_thread_id);
  }

  status = pthread_create(&control_thread_id, NULL, get_stdin, (void *)NULL);
  if (status != 0){
    fprintf(stderr, "pthread_create : %s", strerror(status));
  }
  else{
    printf("[%d]thread_id1 = %d\n", p_pid, control_thread_id);
  }

  pthread_join(control_thread_id, &result);
  printf("[%d]thread_id1 = %d end\n", p_pid, control_thread_id);
  pthread_join(rec_thread_id, &result);
  printf("[%d]thread_id1 = %d end\n", p_pid, rec_thread_id);
  pthread_join(play_thread_id, &result);
  printf("[%d]thread_id2 = %d end\n", p_pid, play_thread_id);

  close(s);

  return 0;
}
