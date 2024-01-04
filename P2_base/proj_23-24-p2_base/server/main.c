#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "common/constants.h"
#include "common/io.h"
#include "operations.h"

int main(int argc, char* argv[]) {
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

  unlink(argv[1]);
  if (mkfifo(argv[1], 0777) < 0) exit(1);
  fprintf(stderr, "Im server initialized the pipe\n");
  if ((svpipe = open(argv[1], O_RDONLY)) < 0) {
    exit(1);
  }
  fprintf(stderr, "Im server opened the pipe\n");
  fprintf(stderr, "Im server ready to read\n");
  read(svpipe, read_from, 40);
  fprintf(stderr, "%s\n", read_from);
  read(svpipe, write_to, 40);
  fprintf(stderr, "%s\n", write_to);
  int in_pipe = open(read_from, O_RDONLY);
  int out_pipe = open(write_to, O_WRONLY);

  // TODO: Intialize server, create worker threads

  while (1) {
    switch (get_code(in_pipe)) {
      case /* constant-expression */:
        /* code */
        break;

      default:
        break;
    }
    // TODO: Read from pipe
    // TODO: Write new client to the producer-consumer buffer
  }

  // TODO: Close Server
  close(svpipe);
  unlink(argv[1]);
  ems_terminate();
}