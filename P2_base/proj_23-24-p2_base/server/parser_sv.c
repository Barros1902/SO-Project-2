#include "parser_sv.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void* get_code(void* list_p) {

  struct QueueList* list = (struct QueueList*)list_p;
  struct Client_data* client_data = dequeue(list);
  int in_pipe = client_data->in_pipe;
  int out_pipe = client_data->out_pipe;
  char op_code;
  int* exit = NULL;
  int result;
  while (1) {
	
    if (read(in_pipe, &op_code, 1) == -1) {
      fprintf(stderr, "failed to read the op_code from the in_pipe");
      return (void*)exit;
    };
    switch (op_code) {
      case OP_CODE_QUIT:;
        free_client_data(client_data);
        struct message_quit message_quit;
        if (read(in_pipe, &message_quit, sizeof(struct message_quit)) == -1) {
          fprintf(stderr, "failed to read the message from in pipe");
          return (void*)exit;
        }
		get_code((void*)list);
        return 0;

      case OP_CODE_CREATE:;
        struct message_create message_create;
        if (read(in_pipe, &message_create, sizeof(struct message_create)) == -1) {
          fprintf(stderr, "failed to read the message from the in_pipe");
          return (void*)exit;
        }
        result = ems_create(message_create.event_id, message_create.num_rows, message_create.num_cols);
        if (write(out_pipe, &result, sizeof(int)) == -1) {
          fprintf(stderr, "failed to write the result to the out_pipe");
          return (void*)exit;
        }
        continue;

      case OP_CODE_RESERVE:;
        struct message_reserve message_reserve;
        size_t* xs = NULL;
        size_t* ys = NULL;
        if (read(in_pipe, &message_reserve, sizeof(struct message_reserve)) == -1) {
          fprintf(stderr, "failed to read the message from the in_pipe");
          return (void*)exit;
        };

        if (message_reserve.num_seats > 0) {
          xs = malloc(message_reserve.num_seats * sizeof(size_t));
          ys = malloc(message_reserve.num_seats * sizeof(size_t));
        }

        if (read(in_pipe, xs, message_reserve.num_seats * sizeof(size_t)) == -1) {
          fprintf(stderr, "failed to read xs from the in_pipe");
          return (void*)exit;
        }
        if (read(in_pipe, ys, message_reserve.num_seats * sizeof(size_t)) == -1) {
          fprintf(stderr, "failed to read ys from the in_pipe");
          return (void*)exit;
        };

        result = ems_reserve(message_reserve.event_id, message_reserve.num_seats, xs, ys);
        if (write(out_pipe, &result, sizeof(int)) == -1) {
          fprintf(stderr, "failed to write the result to the out_pipe");
          return (void*)exit;
        }
        continue;

      case OP_CODE_SHOW:;
        struct message_show message_show;

        size_t rows, cols = 0;
        unsigned int* seats = NULL;

        if (read(in_pipe, &message_show, sizeof(struct message_show)) == -1) {
          fprintf(stderr, "failed to read the message from the in_pipe");
          return (void*)exit;
        }
        result = ems_show_sv(message_show.event_id, &rows, &cols, &seats);

        if (write(out_pipe, &result, sizeof(int)) == -1) {
          fprintf(stderr, "failed to write the result to the out_pipe");
          free(seats);
          return (void*)exit;
        }
        if (write(out_pipe, &rows, sizeof(size_t)) == -1) {
          fprintf(stderr, "failed to write the rows to the out_pipe");
          free(seats);
          return (void*)exit;
        }
        if (write(out_pipe, &cols, sizeof(size_t)) == -1) {
          fprintf(stderr, "failed to write the cols to the out_pipe");
          free(seats);
          return (void*)exit;
        }
        if (write(out_pipe, seats, (rows) * (cols) * sizeof(unsigned int)) == -1) {
          fprintf(stderr, "failed to write the seats to the out_pipe");
          free(seats);
          return (void*)exit;
        }

        free(seats);
        continue;

      case OP_CODE_LIST_EVENTS:;
        size_t num_event = 0;
        unsigned int* ids = NULL;
        result = ems_list_events_sv(&num_event, &ids);

        if (write(out_pipe, &result, sizeof(int)) == -1) {
          fprintf(stderr, "failed to write the result to the out_pipe");
          free(ids);
          return (void*)exit;
        }
        if (write(out_pipe, &num_event, sizeof(size_t)) == -1) {
          fprintf(stderr, "failed to write the number of events to the out_pipe");
          free(ids);
          return (void*)exit;
        }
        if (write(out_pipe, ids, num_event * sizeof(unsigned int)) == -1) {
          fprintf(stderr, "failed to write the ids to the out_pipe");
          free(ids);
          return (void*)exit;
        }

        free(ids);
        continue;

      case '#':

        return (void*)exit;

      case '\n':
        return (void*)exit;

      default:
        fprintf(stderr, "read OP_CODE:%c\n", op_code);
        return (void*)exit;
    }
  }
}
