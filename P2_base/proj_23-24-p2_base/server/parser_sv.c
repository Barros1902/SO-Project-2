#include "parser_sv.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "client/api.h"
#include "common/constants.h"
#include "operations.h"

int get_code(int in_pipe, int out_pipe) {
  char op_code;
  int result;
  while (1) {
    if (read(in_pipe, &op_code, 1) == -1) {
      fprintf(stderr, "failed to read the op_code from the in_pipe");
      return 1;
    };
    switch (op_code) {
      case OP_CODE_QUIT:;
        struct message_quit message_quit;
        if (read(in_pipe, &message_quit, sizeof(struct message_quit)) == -1) {
          fprintf(stderr, "failed to read the message from in pipe");
          return 1;
        }
        return EOC;

      case OP_CODE_CREATE:;
        struct message_create message_create;
        if (read(in_pipe, &message_create, sizeof(struct message_create)) == -1) {
          fprintf(stderr, "failed to read the message from the in_pipe");
          return 1;
        }
        result = ems_create(message_create.event_id, message_create.num_rows, message_create.num_cols);
        if (write(out_pipe, &result, sizeof(int)) == -1) {
          fprintf(stderr, "failed to write the result to the out_pipe");
          return 1;
        }
        continue;

      case OP_CODE_RESERVE:;
        struct message_reserve message_reserve;
        size_t* xs = NULL;
        size_t* ys = NULL;
        if (read(in_pipe, &message_reserve, sizeof(struct message_reserve)) == -1) {
          fprintf(stderr, "failed to read the message from the in_pipe");
          return 1;
        };

        if (message_reserve.num_seats > 0) {
          xs = malloc(message_reserve.num_seats * sizeof(size_t));
          ys = malloc(message_reserve.num_seats * sizeof(size_t));
        }

        if (read(in_pipe, xs, message_reserve.num_seats * sizeof(size_t)) == -1) {
          fprintf(stderr, "failed to read xs from the in_pipe");
          return 1;
        }
        if (read(in_pipe, ys, message_reserve.num_seats * sizeof(size_t)) == -1) {
          fprintf(stderr, "failed to read ys from the in_pipe");
          return 1;
        };

        result = ems_reserve(message_reserve.event_id, message_reserve.num_seats, xs, ys);
        if (write(out_pipe, &result, sizeof(int)) == -1) {
          fprintf(stderr, "failed to write the result to the out_pipe");
          return 1;
        }
        continue;

      case OP_CODE_SHOW:;
        struct message_show message_show;

        size_t rows, cols = 0;
        unsigned int* seats = NULL;

        if (read(in_pipe, &message_show, sizeof(struct message_show)) == -1) {
          fprintf(stderr, "failed to read the message from the in_pipe");
          return 1;
        }
        result = ems_show_sv(out_pipe, message_show.event_id, &rows, &cols, &seats);

        if (write(out_pipe, &result, sizeof(int)) == -1) {
          fprintf(stderr, "failed to write the result to the out_pipe");
          free(seats);
          return 1;
        }
        if (write(out_pipe, &rows, sizeof(size_t)) == -1) {
          fprintf(stderr, "failed to write the rows to the out_pipe");
          free(seats);
          return 1;
        }
        if (write(out_pipe, &cols, sizeof(size_t)) == -1) {
          fprintf(stderr, "failed to write the cols to the out_pipe");
          free(seats);
          return 1;
        }
        if (write(out_pipe, seats, (rows) * (cols) * sizeof(unsigned int)) == -1) {
          fprintf(stderr, "failed to write the seats to the out_pipe");
          free(seats);
          return 1;
        }
        free(seats);
        continue;

      case OP_CODE_LIST_EVENTS:;
        size_t num_event = 0;
        unsigned int* ids = NULL;
        result = ems_list_events_sv(out_pipe, &num_event, &ids);

        if (write(out_pipe, &result, sizeof(int)) == -1) {
          fprintf(stderr, "failed to write the result to the out_pipe");
          free(ids);
          return 1;
        }
        if (write(out_pipe, &num_event, sizeof(size_t)) == -1) {
          fprintf(stderr, "failed to write the number of events to the out_pipe");
          free(ids);
          return 1;
        }
        if (write(out_pipe, ids, num_event * sizeof(unsigned int)) == -1) {
          fprintf(stderr, "failed to write the ids to the out_pipe");
          free(ids);
          return 1;
        }

        free(ids);
        continue;

      case '#':

        return CMD_EMPTY;

      case '\n':
        return CMD_EMPTY;

      default:
        fprintf(stderr, "read OP_CODE:%c\n", op_code);
        return CMD_INVALID;
    }
  }
}
