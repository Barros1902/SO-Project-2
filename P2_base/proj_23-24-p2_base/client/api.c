#include "api.h"

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
  unlink(req_pipe_path);
  unlink(resp_pipe_path);
  mkfifo(req_pipe_path, 0777);
  mkfifo(resp_pipe_path, 0777);
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
  write(pipe_structs.req_pipe, &code, sizeof(char));
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
  char code = OP_CODE_CREATE;
  write(pipe_structs.req_pipe, &code, sizeof(char));
  write(pipe_structs.req_pipe, mensagem, sizeof(struct message_create));
  // send create request to the server (through the request pipe) and wait for the response (through the response
  // pipe)
  return 0;
}

int ems_reserve(unsigned int event_id, size_t num_seats, size_t* xs, size_t* ys) {
  struct message_reserve* mensagem = malloc(sizeof(struct message_reserve));  // TODO free
  if (mensagem != NULL) {
    mensagem->session_id = my_session_id;
    mensagem->event_id = event_id;
    mensagem->num_seats = num_seats;
  } else {
    return 1;
  }
  char code = OP_CODE_RESERVE;
  int done;
  write(pipe_structs.req_pipe, &code, sizeof(char));
  write(pipe_structs.req_pipe, mensagem, sizeof(struct message_reserve));
  write(pipe_structs.req_pipe, xs, num_seats * sizeof(size_t));
  write(pipe_structs.req_pipe, ys, num_seats * sizeof(size_t));
  read(pipe_structs.resp_pipe, &done, sizeof(int));

  // send reserve request to the server (through the request pipe) and wait for the response (through the response
  // pipe)
  return 0;
}

int ems_show(int out_fd, unsigned int event_id) {
  struct message_show* mensagem = malloc(sizeof(struct message_show));  // TODO free
  size_t cols = 0;
  size_t rows = 0;
  ssize_t read_num = 0;
  unsigned int* seats;
  int result;

  if (mensagem != NULL) {
    mensagem->session_id = my_session_id;
    mensagem->event_id = event_id;
  } else {
    return 1;
  }

  fprintf(stderr, "Inside show");
  char code = OP_CODE_SHOW;
  write(pipe_structs.req_pipe, &code, sizeof(char));
  write(pipe_structs.req_pipe, mensagem, sizeof(struct message_show));
  read_num = read(pipe_structs.resp_pipe, &result, sizeof(int));

  if (read_num < 1) {
    fprintf(stderr, "não funcionou");
    return 1;
  }

  read(pipe_structs.resp_pipe, &rows, sizeof(size_t));
  read(pipe_structs.resp_pipe, &cols, sizeof(size_t));
  seats = malloc(sizeof(unsigned int) * (rows) * (cols));
  read(pipe_structs.resp_pipe, seats, sizeof(unsigned int) * (rows) * (cols));

  fprintf(stderr, "\n%ld - rows\n %ld - cols\n", rows, cols);
  for (size_t i = 1; i <= rows; i++) {
    for (size_t j = 1; j <= cols; j++) {
      unsigned int seat_value = seats[(cols * (i - 1)) + (j - 1)];

      // Convert unsigned int to string
      char seat_string[20];  // Adjust size as per your maximum expected value
      sprintf(seat_string, "%u", seat_value);

      // Write string representation of seat_value to out_fd
      write(2, seat_string, strlen(seat_string));

      if (j < cols) {
        char space = ' ';
        write(2, &space, sizeof(char));
      }
    }

    char newline = '\n';
    write(2, &newline, sizeof(char));
  }

  // send show request to the server (through the request pipe) and wait for the response (through the response
  // pipe)
  return 0;
}

int ems_list_events(int out_fd) {
  char code = OP_CODE_RESERVE;
  write(pipe_structs.req_pipe, &code, sizeof(char));

  // if (current == NULL) {
  //   char buff[] = "No events\n";
  //   if (print_str(out_fd, buff)) {
  //     perror("Error writing to file descriptor");
  //     pthread_rwlock_unlock(&event_list->rwl);
  //     return 1;
  //   }

  //   pthread_rwlock_unlock(&event_list->rwl);
  //   return 0;
  // }

  // while (1) {
  //   char buff[] = "Event: ";
  //   if (print_str(out_fd, buff)) {
  //     perror("Error writing to file descriptor");
  //     pthread_rwlock_unlock(&event_list->rwl);
  //     return 1;
  //   }

  //   char id[16];
  //   sprintf(id, "%u\n", (current->event)->id);
  //   if (print_str(out_fd, id)) {
  //     perror("Error writing to file descriptor");
  //     pthread_rwlock_unlock(&event_list->rwl);
  //     return 1;
  //   }

  //   if (current == to) {
  //     break;
  //   }

  //   current = current->next;
  // }
  
  // TODO: send list request to the server (through the request pipe) and wait for the response (through the response
  // pipe)
  return 1;
}
