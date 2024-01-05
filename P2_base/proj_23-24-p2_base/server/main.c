#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "common/constants.h"
#include "common/io.h"
#include "operations.h"
#include "parser_sv.h"
#include "sessions.h"
int sigusr1_received;

void signal_handler(int sig) {
  if (signal(sig, signal_handler)) {
    exit(EXIT_FAILURE);
  }
  sigusr1_received = 1;
}

int main(int argc, char* argv[]) {
  signal(SIGUSR1, signal_handler);
  if (argc < 2 || argc > 3) {
    fprintf(stderr, "Usage: %s\n <pipe_path> [delay]\n", argv[0]);
    return 1;
  }

  char* endptr;
  unsigned int state_access_delay_us = STATE_ACCESS_DELAY_US;
  if (argc == 3) {
    unsigned long int delay = strtoul(argv[2], &endptr, 10);

    if (*endptr != '\0' || delay > UINT_MAX) {
      fprintf(stderr, "Invalid delay value or value too large\n");
      return 1;
    }

    state_access_delay_us = (unsigned int)delay;
  }

  if (ems_init(state_access_delay_us)) {
    fprintf(stderr, "Failed to initialize EMS\n");
    return 1;
  }
  int svpipe;
  char read_from[40];
  char write_to[40];
  int session_id = 0;
  struct QueueList* list = create_queue();
  unlink(argv[1]);
  if (mkfifo(argv[1], 0777) < 0) exit(1);
  if ((svpipe = open(argv[1], O_RDWR)) < 0) {
    exit(1);
  }

  pthread_t tid[THREADS_AMOUNT];
  for (int i = 0; i < THREADS_AMOUNT; i++) {
    pthread_create(&tid[i], NULL, get_code, (void*)list);
  }

  while (1) {
    if (sigusr1_received) {
      sigusr1_received = 0;
      ems_show_all();
    }
    struct Client_data* data = malloc(sizeof(struct Client_data));
    read(svpipe, read_from, 40);
    read(svpipe, write_to, 40);
    int session_id_giver = session_id % THREADS_AMOUNT;
    int in_pipe = open(read_from, O_RDONLY);
    int out_pipe = open(write_to, O_WRONLY);
    write(out_pipe, &session_id_giver, sizeof(int));
    session_id++;
    strcpy(data->in_pipe_path, read_from);
    strcpy(data->out_pipe_path, write_to);
    data->in_pipe = in_pipe;
    data->out_pipe = out_pipe;
    queue_data(list, data);
  }

  // TODO: Intialize server, create worker threads

  // TODO: Close Server
  close(svpipe);
  unlink(argv[1]);
  ems_terminate();
}