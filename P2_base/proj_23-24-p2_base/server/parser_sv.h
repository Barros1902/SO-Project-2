#include "sessions.h"
#include "client/api.h"
#include "common/constants.h"
#include "operations.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


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

void* get_code(void* list);