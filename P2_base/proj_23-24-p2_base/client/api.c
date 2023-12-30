#include "api.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>


int ems_setup(char const* req_pipe_path, char const* resp_pipe_path, char const* server_pipe_path) {
	int server_pipe_path_, written, req_pipe, resp_pipe;
	char req_len[10000];
	sprintf(req_len, "%dG", (int)strlen(req_pipe_path));
	unlink(req_pipe_path);
	unlink(resp_pipe_path);
	mkfifo(req_pipe_path, 0777);
	mkfifo(resp_pipe_path, 0777);
	if (!(req_pipe = open(req_pipe_path, O_WRONLY))){
		return 1;
	}

	if (!(resp_pipe = open(resp_pipe_path, O_RDONLY))){
		return 1;
	}


	if (!(server_pipe_path_ = open(server_pipe_path, O_WRONLY))){
		return 1;
	}
	printf("%s - req_pipe\n%s - resp_pipe\n%s - reqln\n", req_pipe_path, resp_pipe_path, req_len);
	scanf("%d", &written);
	written = write(server_pipe_path_, strcat(strcat(req_len, req_pipe_path), resp_pipe_path), strlen(req_len) + strlen(req_pipe_path) + strlen(resp_pipe_path) + 1);
	if (written == -1){
		return 1;
	}
}

int ems_quit(void) { 
  //TODO: close pipes
  return 1;
}

int ems_create(unsigned int event_id, size_t num_rows, size_t num_cols) {
  //TODO: send create request to the server (through the request pipe) and wait for the response (through the response pipe)
  return 1;
}

int ems_reserve(unsigned int event_id, size_t num_seats, size_t* xs, size_t* ys) {
  //TODO: send reserve request to the server (through the request pipe) and wait for the response (through the response pipe)
  return 1;
}

int ems_show(int out_fd, unsigned int event_id) {
  //TODO: send show request to the server (through the request pipe) and wait for the response (through the response pipe)
  return 1;
}

int ems_list_events(int out_fd) {
  //TODO: send list request to the server (through the request pipe) and wait for the response (through the response pipe)
  return 1;
}
