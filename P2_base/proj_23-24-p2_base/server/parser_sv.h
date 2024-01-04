enum Command {
  CMD_CREATE,
  CMD_RESERVE,
  CMD_SHOW,
  CMD_LIST_EVENTS,
  CMD_WAIT,
  CMD_HELP,
  CMD_EMPTY,
  CMD_INVALID,
  EOC  // End of commands
};

int get_code(int in_pipe, int out_pipe);