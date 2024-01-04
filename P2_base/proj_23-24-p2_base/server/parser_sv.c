#include "common/constants.h"
#include "parser_sv.h"
#include "client/api.h"

int get_code(int in_pipe, int out_pipe) {
  while(1){
    char op_code;
    read(in_pipe, op_code, 1);
    switch (op_code) {

      case OP_CODE_QUIT:
        struct message_quit message_quit;
        read(in_pipe,message_quit,sizeof(struct message_quit));
        return EOC;

      case OP_CODE_CREATE:
        struct message_create message_create;
        read(in_pipe, message_create, sizeof(struct message_create));
        write(out_pipe, ems_create(message_create.event_id, message_create.num_rows, message_create.num_cols), sizeof(int));
        continue;

      case OP_CODE_RESERVE:
        struct message_reserve message_reserve;
        read(in_pipe, message_reserve, sizeof(struct message_reserve));
        write(out_pipe,ems_reserve(message_reserve.event_id, message_reserve.num_seats, message_reserve.xs, message_reserve.ys) , sizeof(int));
        continue;

      case OP_CODE_SHOW:
        struct message_show message_show;
        read(in_pipe, message_show, sizeof(struct message_show));
        write(out_pipe, ems_show(out_pipe, message_show.event_id), sizeof(int));
        
        return ;

      case OP_CODE_LIST_EVENTS:

        

        return CMD_HELP;

      case '#':
        cleanup(in_pipe);
        return CMD_EMPTY;

      case '\n':
        return CMD_EMPTY;

      default:
        cleanup(in_pipe);
        return CMD_INVALID;
    }
  }
}