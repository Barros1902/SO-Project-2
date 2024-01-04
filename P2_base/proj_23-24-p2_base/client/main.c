#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


#include "api.h"
#include "common/constants.h"
#include "parser.h"

int main(int argc, char* argv[]) {
  if (argc < 5) {
    fprintf(stderr, "Usage: %s <request pipe path> <response pipe path> <server pipe path> <.jobs file path>\n",
            argv[0]);
    return 1;
  }

  if (ems_setup(argv[1], argv[2], argv[3])) {
    fprintf(stderr, "Failed to set up EMS\n");
    return 1;
  }

  const char* dot = strrchr(argv[4], '.');
  if (dot == NULL || dot == argv[4] || strlen(dot) != 5 || strcmp(dot, ".jobs") ||
      strlen(argv[4]) > MAX_JOB_FILE_NAME_SIZE) {
    fprintf(stderr, "The provided .jobs file path is not valid. Path: %s\n", argv[1]);
    return 1;
  }

  char out_path[MAX_JOB_FILE_NAME_SIZE];
  strcpy(out_path, argv[4]);
  strcpy(strrchr(out_path, '.'), ".out");

  int in_fd = open(argv[4], O_RDONLY);
  if (in_fd == -1) {
    fprintf(stderr, "Failed to open input file. Path: %s\n", argv[4]);
    return 1;
  }

  int out_fd = open(out_path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if (out_fd == -1) {
    fprintf(stderr, "Failed to open output file. Path: %s\n", out_path);
    return 1;
  }
  parse_start(in_fd, out_fd);

}