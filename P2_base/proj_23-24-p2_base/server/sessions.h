#ifndef SESSIONS_H
#define SESSIONS_H

#include <pthread.h>
#include <stddef.h>

struct Client_data {
  char in_pipe_path[40];
  char out_pipe_path[40];
  int in_pipe;
  int out_pipe;

};

struct QueueNode {
  struct Client_data* data;
  struct QueueNode* next;
};

// Linked list structure
struct QueueList {
  struct QueueNode* head;  	// Head of the queue
  struct QueueNode* tail;  	// Tail of the queue
  pthread_mutex_t mutex;   	// Mutex to protect the queue
  pthread_cond_t condition; // Condition variable
};

struct QueueList* create_queue();
int queue_data(struct QueueList* list, struct Client_data* data);
void free_client_data(struct Client_data* data);
void free_queue(struct QueueList* list);
struct Client_data* dequeue(struct QueueList* list);

#endif  // SESSIONS_H