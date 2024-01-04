#include "api.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static struct pipe_struct pipe_structs;
size_t my_session_id;
ssize_t written;
int ems_setup(char const* req_pipe_path, char const* resp_pipe_path, char const* server_pipe_path) {
  int server_pipe;
  unlink(req_pipe_path);
  unlink(resp_pipe_path);
  mkfifo(req_pipe_path, 0777);
  mkfifo(resp_pipe_path, 0777);
  strcpy(pipe_structs.req_pipe_path, req_pipe_path);
  strcpy(pipe_structs.resp_pipe_path, resp_pipe_path);
  if (!(server_pipe = open(server_pipe_path, O_WRONLY))) {
    return 1;
  }
  printf("%s - req_pipe\n%s - resp_pipe\n", req_pipe_path, resp_pipe_path);
  written = write(server_pipe, req_pipe_path, PIPE_NAME_SIZE);
  if (written == -1) {
    return 1;
  }
  written = write(server_pipe, resp_pipe_path, PIPE_NAME_SIZE);
  if (written == -1) {
    return 1;
  }

  if (!(pipe_structs.req_pipe = open(req_pipe_path, O_WRONLY))) {
    return 1;
  }

  if (!(pipe_structs.resp_pipe = open(resp_pipe_path, O_RDONLY))) {
    return 1;
  }

  if (read(pipe_structs.resp_pipe, &my_session_id, sizeof(size_t)) == -1) {
    return 1;
  }

  printf("Didas");
  return 0;
}

int ems_quit(void) {
  struct message_quit* mensagem = malloc(sizeof(struct message_quit));  // TODO free
  if (mensagem != NULL) {
    mensagem->session_id = my_session_id;
  } else {
    return 1;
  }
  write(pipe_structs.req_pipe, OP_CODE_QUIT, sizeof(char));
  write(pipe_structs.req_pipe, mensagem, sizeof(struct message_quit));
  close(pipe_structs.req_pipe);
  close(pipe_structs.resp_pipe);

  if (unlink(pipe_structs.req_pipe_path) != 0) {
    fprintf(stderr, "[ERR]: unlink(%s) failed\n", pipe_structs.req_pipe_path);
    return 1;
  }
  if (unlink(pipe_structs.resp_pipe_path) != 0) {
    fprintf(stderr, "[ERR]: unlink(%s) failed\n", pipe_structs.resp_pipe_path);
    return 1;
  }

  return 0;
}

int ems_create(unsigned int event_id, size_t num_rows, size_t num_cols) {
  struct message_create* mensagem = malloc(sizeof(struct message_create));  // TODO free

  if (mensagem != NULL) {
    mensagem->session_id = my_session_id;
    mensagem->event_id = event_id;
    mensagem->num_rows = num_rows;
    mensagem->num_cols = num_cols;
  } else {
    return 1;
  }
  write(pipe_structs.req_pipe, OP_CODE_CREATE, sizeof(char));
  write(pipe_structs.req_pipe, mensagem, sizeof(struct message_create));
  // send create request to the server (through the request pipe) and wait for the response (through the response
  // pipe)
  return 1;
}

int ems_reserve(unsigned int event_id, size_t num_seats, size_t* xs, size_t* ys) {
  struct message_reserve* mensagem = malloc(sizeof(struct message_reserve));  // TODO free
  if (mensagem != NULL) {
    mensagem->session_id = my_session_id;
    mensagem->event_id = event_id;
    mensagem->num_seats = num_seats;
    mensagem->xs = xs;
    mensagem->ys = ys;
  } else {
    return 1;
  }
  write(pipe_structs.req_pipe, OP_CODE_RESERVE, sizeof(char));
  write(pipe_structs.req_pipe, mensagem, sizeof(struct message_reserve));

  // send reserve request to the server (through the request pipe) and wait for the response (through the response
  // pipe)
  return 1;
}

int ems_show(int out_fd, unsigned int event_id) {
  struct message_show* mensagem = malloc(sizeof(struct message_show));  // TODO free
  size_t* cols;
  size_t* rows;
  unsigned int* seats;

  cols = malloc(sizeof(size_t));
  rows = malloc(sizeof(size_t));

  if (mensagem != NULL) {
    mensagem->session_id = my_session_id;
    mensagem->event_id = event_id;
  } else {
    return 1;
  }
  write(pipe_structs.req_pipe, OP_CODE_SHOW, sizeof(char));
  write(pipe_structs.req_pipe, mensagem, sizeof(struct message_show));
  read(pipe_structs.resp_pipe, rows, sizeof(size_t));
  read(pipe_structs.resp_pipe, cols, sizeof(size_t));
  seats = malloc(sizeof(unsigned int) * (*rows) * (*cols));
  read(pipe_structs.resp_pipe, seats, sizeof(*rows * (*cols)));
  for (size_t i = 1; i <= *rows; i++) {
    for (size_t j = 1; j <= *cols; j++) {
      unsigned int seat_value = seats[(*cols * (i - 1)) + (j - 1)];

      // Convert unsigned int to string
      char seat_string[20];  // Adjust size as per your maximum expected value
      sprintf(seat_string, "%u", seat_value);

      // Write string representation of seat_value to out_fd
      write(out_fd, seat_string, strlen(seat_string));

      if (j < *cols) {
        char space = ' ';
        write(out_fd, &space, sizeof(char));
      }
    }

    char newline = '\n';
    write(out_fd, &newline, sizeof(char));
  }
  // send show request to the server (through the request pipe) and wait for the response (through the response
  // pipe)
  return 1;
}

int ems_list_events(int out_fd) {
  // TODO: send list request to the server (through the request pipe) and wait for the response (through the response
  // pipe)
  return 1;
}
