#include "api.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static struct pipe_struct pipe_structs;
int my_session_id;
ssize_t written;
int ems_setup(char const* req_pipe_path, char const* resp_pipe_path, char const* server_pipe_path) {
  int server_pipe;

  if (unlink(req_pipe_path) != 0 && errno != ENOENT) {
    fprintf(stderr, "[ERR]: unlink(%s) failed: %s\n", req_pipe_path, strerror(errno));
    exit(EXIT_FAILURE);
  }
  if (unlink(resp_pipe_path) != 0 && errno != ENOENT) {
    fprintf(stderr, "[ERR]: unlink(%s) failed: %s\n", resp_pipe_path, strerror(errno));
    exit(EXIT_FAILURE);
  }

  if (mkfifo(req_pipe_path, 0777) == -1) {
    fprintf(stderr, "Error creating req_pipe_path: %s\n", strerror(errno));
    return 1;
  }

  if (mkfifo(resp_pipe_path, 0777) == -1) {
    fprintf(stderr, "Error creating req_pipe_path: %s\n", strerror(errno));
    return 1;
  }

  strcpy(pipe_structs.req_pipe_path, req_pipe_path);
  strcpy(pipe_structs.resp_pipe_path, resp_pipe_path);

  if (!(server_pipe = open(server_pipe_path, O_WRONLY))) {
    return 1;
  }
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

  if (read(pipe_structs.resp_pipe, &my_session_id, sizeof(int)) == -1) {
	
    return 1;
  }
  return 0;
}

int ems_quit(void) {
  struct message_quit* mensagem = malloc(sizeof(struct message_quit));  // TODO free
  if (mensagem != NULL) {
    mensagem->session_id = my_session_id;
  } else {
    return 1;
  }
  char code = OP_CODE_QUIT;
  if (write(pipe_structs.req_pipe, &code, sizeof(char)) == -1) {
    perror("Error writing op_code\n");
    exit(EXIT_FAILURE);
  }
  if (write(pipe_structs.req_pipe, mensagem, sizeof(struct message_quit)) == -1) {
    perror("Error writing session id\n");
    exit(EXIT_FAILURE);
  }
  if (close(pipe_structs.req_pipe) == -1) perror("Error closing req_pipe");

  if (close(pipe_structs.resp_pipe) == -1) perror("Error closing reso_pipe");
  ;

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

// send create request to the server (through the request pipe) and wait for the response (through the response pipe)
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
  char code = OP_CODE_CREATE;
  int done = 0;
  ssize_t read_num = 0;
  if (write(pipe_structs.req_pipe, &code, sizeof(char)) == -1) {
    fprintf(stderr, "failed do write op_code to req_pipe");
    free(mensagem);
    return 1;
  }

  if (write(pipe_structs.req_pipe, mensagem, sizeof(struct message_create)) == -1) {
    fprintf(stderr, "failed do write op_code to req_pipe");
    free(mensagem);
    return 1;
  };
  read_num = read(pipe_structs.resp_pipe, &done, sizeof(int));
  if (read_num < 1) {
    fprintf(stderr, "read failed 1");
    free(mensagem);
    return 1;
  }

  free(mensagem);
  return 0;
}

// send reserve request to the server (through the request pipe) and wait for the response (through the response pipe)
int ems_reserve(unsigned int event_id, size_t num_seats, size_t* xs, size_t* ys) {
  struct message_reserve* mensagem = malloc(sizeof(struct message_reserve));
  if (mensagem != NULL) {
    mensagem->session_id = my_session_id;
    mensagem->event_id = event_id;
    mensagem->num_seats = num_seats;
  } else {
    return 1;
  }
  char code = OP_CODE_RESERVE;
  int done;
  ssize_t read_num = 0;

  if (write(pipe_structs.req_pipe, &code, sizeof(char)) == -1) {
    fprintf(stderr, "failed to write op_code to req_pipe");
    free(mensagem);
    return 1;
  }

  if (write(pipe_structs.req_pipe, mensagem, sizeof(struct message_reserve)) == -1) {
    fprintf(stderr, "failed to write the struct to req_pipe");
    free(mensagem);
    return 1;
  }

  if (write(pipe_structs.req_pipe, xs, num_seats * sizeof(size_t)) == -1) {
    fprintf(stderr, "failed to write xs to req_pipe");
    free(mensagem);
    return 1;
  }

  if (write(pipe_structs.req_pipe, ys, num_seats * sizeof(size_t)) == -1) {
    fprintf(stderr, "failed to write ys to req_pipe");
    free(mensagem);
    return 1;
  }

  read_num = read(pipe_structs.resp_pipe, &done, sizeof(int));

  if (read_num < 1) {
    fprintf(stderr, "read failed 2");
    free(mensagem);
    return 1;
  }

  free(mensagem);
  return 0;
}

// send show request to the server (through the request pipe) and wait for the response (through the response pipe)
int ems_show(int out_fd, unsigned int event_id) {
  struct message_show* mensagem = malloc(sizeof(struct message_show));
  size_t cols = 0;
  size_t rows = 0;
  ssize_t read_num = 0;
  unsigned int* seats;
  int result = 0;

  if (mensagem != NULL) {
    mensagem->session_id = my_session_id;
    mensagem->event_id = event_id;
  } else {
    return 1;
  }

  char code = OP_CODE_SHOW;

  if (write(pipe_structs.req_pipe, &code, sizeof(char)) == -1) {
    fprintf(stderr, "failed to write op_code to req_pipe");
    free(mensagem);
    return 1;
  }
  if (write(pipe_structs.req_pipe, mensagem, sizeof(struct message_show)) == -1) {
    fprintf(stderr, "failed to write the struct mensagem to req_pipe");
    free(mensagem);
    return 1;
  }
  read_num = read(pipe_structs.resp_pipe, &result, sizeof(int));


  if (read_num < 1) {
    fprintf(stderr, "read failed 3");
    free(mensagem);
    return 1;
  }

  if (read(pipe_structs.resp_pipe, &rows, sizeof(size_t)) == -1) {
    fprintf(stderr, "failed to read rows from resp_pipe");
    free(mensagem);
    return 1;
  }

  if (read(pipe_structs.resp_pipe, &cols, sizeof(size_t)) == -1) {
    fprintf(stderr, "failed to read cols from resp_pipe");
    free(mensagem);
    return 1;
  }

  seats = malloc(sizeof(unsigned int) * (rows) * (cols));
  if (read(pipe_structs.resp_pipe, seats, sizeof(unsigned int) * (rows) * (cols)) == -1) {
    fprintf(stderr, "failed to read seats from resp_pipe");
    free(mensagem);
    return 1;
  };

  for (size_t i = 1; i <= rows; i++) {
    for (size_t j = 1; j <= cols; j++) {
      unsigned int seat_value = seats[(cols * (i - 1)) + (j - 1)];

      char seat_string[20];
      sprintf(seat_string, "%u", seat_value);

      // Write string representation of seat_value to out_fd
      if (write(out_fd, seat_string, strlen(seat_string)) == -1) {
        fprintf(stderr, "failed to write the seat string to out_fd");
        free(mensagem);
        return 1;
      };

      if (j < cols) {
        char space = ' ';
        if (write(out_fd, &space, sizeof(char)) == -1) {
          fprintf(stderr, "failed to write the space to out_fd");
          free(mensagem);
          return 1;
        };
      }
    }

    char newline = '\n';
    if (write(out_fd, &newline, sizeof(char)) == -1) {
      fprintf(stderr, "failed to write the newlinw to out_fd");
      free(mensagem);
      return 1;
    };
  }

  free(mensagem);
  return 0;
}

int ems_list_events(int out_fd) {
  char code = OP_CODE_LIST_EVENTS;
  size_t num_event;
  int result = 0;
  ssize_t read_num = 0;
  unsigned int* ids;

  if (write(pipe_structs.req_pipe, &code, sizeof(char)) == -1) {
    fprintf(stderr, "failed to write the op_code to the req_pipe");
    return 1;
  }
  read_num = read(pipe_structs.resp_pipe, &result, sizeof(int));

  if (read_num < 1) {
    fprintf(stderr, "read failed 4");
    return 1;
  }

  if (read(pipe_structs.resp_pipe, &num_event, sizeof(size_t)) != sizeof(size_t)) {
    perror("Error reading num_event");
    return 1;
  }
  if (num_event == 0) {
    // No events found, write appropriate message to out_fd
    const char no_events[] = "No events\n";
    if (write(out_fd, no_events, strlen(no_events)) == -1) {
      perror("Error writing to file descriptor");
      return 1;
    }
    return 0;
  }

  ids = malloc(num_event * sizeof(unsigned int));

  if (ids == NULL) {
    perror("Memory allocation failed for ids");
    return 1;
  }

  if (read(pipe_structs.resp_pipe, ids, num_event * sizeof(unsigned int)) !=
      (ssize_t)(num_event * sizeof(unsigned int))) {
    perror("Error reading ids");
    free(ids);
    return 1;
  }

  for (size_t i = 0; i < num_event; ++i) {
    char event_str[16];
    sprintf(event_str, "Event: %u\n", ids[i]);
    if (write(out_fd, event_str, strlen(event_str)) == -1) {
      perror("Error writing to file descriptor");
      free(ids);
      return 1;
    }
  }

  free(ids);
  return 0;
}
