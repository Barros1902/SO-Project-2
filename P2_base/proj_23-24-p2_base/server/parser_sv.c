#include "parser_sv.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "client/api.h"
#include "common/constants.h"

int get_code(int in_pipe, int out_pipe) {
  char op_code;
  int result;
  while (1) {
    fprintf(stderr, "Server waiting to read an op_code\n");
    read(in_pipe, &op_code, 1);
    fprintf(stderr, "%c", op_code);
    switch (op_code) {
      case OP_CODE_QUIT:
        struct message_quit message_quit;
        read(in_pipe, &message_quit, sizeof(struct message_quit));
        return EOC;

      case OP_CODE_CREATE:
        struct message_create message_create;
        read(in_pipe, &message_create, sizeof(struct message_create));
        result = ems_create(message_create.event_id, message_create.num_rows, message_create.num_cols);
        write(out_pipe, &result, sizeof(int));
        continue;

      case OP_CODE_RESERVE:
        struct message_reserve message_reserve;
        size_t* xs = NULL;
        size_t* ys = NULL;
        fprintf(stderr, "here");
        read(in_pipe, &message_reserve, sizeof(struct message_reserve));

        if (message_reserve.num_seats > 0) {
          xs = malloc(message_reserve.num_seats * sizeof(size_t));
          ys = malloc(message_reserve.num_seats * sizeof(size_t));
        }

        read(in_pipe, xs, message_reserve.num_seats * sizeof(size_t));
        read(in_pipe, ys, message_reserve.num_seats * sizeof(size_t));
        fprintf(stderr, "%u - event id %zu - num_seats", message_reserve.event_id, message_reserve.num_seats);
        for (int i = 0; i < (int)message_reserve.num_seats; i++) {
          fprintf(stderr, "%zu ", xs[i]);
        }
        fprintf(stderr, "- xs\n");
        for (int i = 0; i < (int)message_reserve.num_seats; i++) {
          fprintf(stderr, "%zu ", ys[i]);
        }
		fprintf(stderr, "- xs\n");
        result = ems_reserve(message_reserve.event_id, message_reserve.num_seats, xs, ys);
        fprintf(stderr, "estou no reserve\n");
        write(out_pipe, &result, sizeof(int));
        continue;

      case OP_CODE_SHOW:
        struct message_show message_show;
        read(in_pipe, &message_show, sizeof(struct message_show));
        result = ems_show(out_pipe, message_show.event_id);
        write(out_pipe, &result, sizeof(int));

        continue;

      case OP_CODE_LIST_EVENTS:

        return CMD_HELP;

      case '#':

        return CMD_EMPTY;

      case '\n':
        return CMD_EMPTY;

      default:

        return CMD_INVALID;
    }
  }
}
