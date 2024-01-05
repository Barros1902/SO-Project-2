#define MAX_RESERVATION_SIZE 256
#define STATE_ACCESS_DELAY_US 500000  // 500ms
#define MAX_JOB_FILE_NAME_SIZE 256
#define MAX_SESSION_COUNT 8
#define MAX_PIPE_PATH 256
#define MESSAGE_SIZE 1024
#define PIPE_NAME_SIZE 40
#define THREADS_AMOUNT 8

#define OP_CODE_SETUP '1'
#define OP_CODE_QUIT '2'
#define OP_CODE_CREATE '3'
#define OP_CODE_RESERVE '4'
#define OP_CODE_SHOW '5'
#define OP_CODE_LIST_EVENTS '6'
