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
    read(in_pipe, &op_code, 1);
    fprintf(stderr, "OP_CODE %c\n", op_code);
    switch (op_code) {
      case OP_CODE_QUIT:
        ;
        struct message_quit message_quit;
        read(in_pipe, &message_quit, sizeof(struct message_quit));
        return EOC;

      case OP_CODE_CREATE:
        ;
        struct message_create message_create;
        read(in_pipe, &message_create, sizeof(struct message_create));
        result = ems_create(message_create.event_id, message_create.num_rows, message_create.num_cols);
        write(out_pipe, &result, sizeof(int));
        continue;

      case OP_CODE_RESERVE:
        ;
        struct message_reserve message_reserve;
        size_t* xs = NULL;
        size_t* ys = NULL;
        read(in_pipe, &message_reserve, sizeof(struct message_reserve));
        fprintf(stderr, "here %d\n",(int) message_reserve.num_seats);

        if (message_reserve.num_seats > 0) {
          xs = malloc(message_reserve.num_seats * sizeof(size_t));
          ys = malloc(message_reserve.num_seats * sizeof(size_t));
        }

        read(in_pipe, xs, message_reserve.num_seats * sizeof(size_t));
        read(in_pipe, ys, message_reserve.num_seats * sizeof(size_t));
        
        result = ems_reserve(message_reserve.event_id, message_reserve.num_seats, xs, ys);
        write(out_pipe, &result, sizeof(int));

        continue;

      case OP_CODE_SHOW:
        ;
        struct message_show message_show;

        size_t rows, cols = 0;
        unsigned int* seats = NULL;

        read(in_pipe, &message_show, sizeof(struct message_show));
        result = ems_show_sv(out_pipe, message_show.event_id, &rows, &cols, &seats);
        printf("rows - %ld\n cols - %ld\n", rows, cols);
        write(out_pipe, &result, sizeof(int));
        write(out_pipe, &rows, sizeof(size_t));
        write(out_pipe, &cols, sizeof(size_t));
        write(out_pipe, seats, (rows) * (cols) * sizeof(unsigned int));
        free(seats);
        continue;

      case OP_CODE_LIST_EVENTS:
        ;
        size_t num_event = 0;
        unsigned int* ids = NULL;
        result = ems_list_events_sv(out_pipe, &num_event, &ids);
  
        write(out_pipe,&result, sizeof(int));
        write(out_pipe,&num_event, sizeof(size_t));
        write(out_pipe,ids, num_event * sizeof(unsigned int));
        
        free(ids);
        continue;

      case '#':

        return CMD_EMPTY;

      case '\n':
        return CMD_EMPTY;

      default:
        fprintf(stderr,"read OP_CODE:%c\n",op_code);
        return CMD_INVALID;
    }
  }
}
