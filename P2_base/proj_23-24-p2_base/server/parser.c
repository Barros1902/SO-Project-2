#include "common/constants.h"
#include "parser.h"
#include "client/api.h"

int get_code(int in_pipe) {
   char op_code;
  if (read(in_pipe, op_code, 1) != 1) {
    return EOC;
  }

  switch (op_code) {
    case OP_CODE_SETUP:
      if (read(in_pipe, buf + 1, 6) != 6 || strncmp(buf, "CREATE ", 7) != 0) {
        cleanup(in_pipe);
        return CMD_INVALID;
      }

      return CMD_CREATE;

    case OP_CODE_QUIT:
      struct message_quit message_quit;


      return CMD_RESERVE;

    case OP_CODE_CREATE:
      struct message_create message_create;
      ems_create(message_create.event_id, message_create.num_rows, message_create.num_cols);

      return CMD_SHOW;

    case OP_CODE_RESERVE:
      if (read(in_pipe, buf + 1, 3) != 3 || strncmp(buf, "LIST", 4) != 0) {
        cleanup(in_pipe);
        return CMD_INVALID;
      }

      if (read(in_pipe, buf + 4, 1) != 0 && buf[4] != '\n') {
        cleanup(in_pipe);
        return CMD_INVALID;
      }

      return CMD_LIST_EVENTS;

    case OP_CODE_SHOW:
      if (read(in_pipe, buf + 1, 4) != 4 || strncmp(buf, "WAIT ", 5) != 0) {
        cleanup(in_pipe);
        return CMD_INVALID;
      }

      return CMD_WAIT;

    case OP_CODE_LIST_EVENTS:
      if (read(in_pipe, buf + 1, 3) != 3 || strncmp(buf, "HELP", 4) != 0) {
        cleanup(in_pipe);
        return CMD_INVALID;
      }

      if (read(in_pipe, buf + 4, 1) != 0 && buf[4] != '\n') {
        cleanup(in_pipe);
        return CMD_INVALID;
      }

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