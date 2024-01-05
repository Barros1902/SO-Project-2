#include "sessions.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

struct QueueList* create_queue() {
  struct QueueList* list = (struct QueueList*)malloc(sizeof(struct QueueList));
  if (!list) return NULL;
  list->head = NULL;
  list->tail = NULL;
  if (pthread_mutex_init(&list->mutex, NULL) != 0) {
	fprintf(stderr, "GAY\n\n\n");
    perror("Error initializing list_mutex");
  }
  if (pthread_cond_init(&list->condition, NULL) != 0) {
    perror("Error initializing list_condition_variable");
  }
  return list;
}

int queue_data(struct QueueList* list, struct Client_data* data) {

  if (!list) {
    return 1;
  }
  if ((pthread_mutex_lock(&(list->mutex))) != 0) {
    perror("Error locking list_mutex while queueing");
  }
  struct QueueNode* new_node = (struct QueueNode*)malloc(sizeof(struct QueueNode));
  if (!new_node) {
    pthread_mutex_unlock(&(list->mutex));
    perror("Error allocating memory for the new node");
    return 1;
  }

  new_node->data = data;
  new_node->next = NULL;

  if (list->head == NULL) {
    list->head = new_node;
    list->tail = new_node;
  } else {
    list->tail->next = new_node;
    list->tail = new_node;
  }
  pthread_cond_signal(&(list->condition));
  pthread_mutex_unlock(&(list->mutex));
  return 0;
}

void free_client_data(struct Client_data* data) {
  if (!data) return;
  free(data);
}

void free_queue(struct QueueList* list) {
  if (!list) return;

  struct QueueNode* current = list->head;
  while (current) {
    struct QueueNode* temp = current;
    current = current->next;

    free_client_data(temp->data);
    free(temp);
    pthread_cond_destroy(&(list->condition));
    pthread_mutex_destroy(&(list->mutex));
  }

  free(list);
}

struct Client_data* dequeue(struct QueueList* list) {
  if ((pthread_mutex_lock(&list->mutex)) != 0) {
    fprintf(stderr, "Error locking list_mutex while dequeueing");
  }
  if (!list) {
    pthread_mutex_unlock(&(list->mutex));
    return NULL;
  }
  

  while (!list->head) {
    pthread_cond_wait(&(list->condition), &(list->mutex));

  }
	struct QueueNode* current = list->head;
    struct Client_data* data = current->data;
  if (!current->next) {
    free(current);
	list->head = NULL;
    pthread_mutex_unlock(&(list->mutex));
	
    return data;
  }
  list->head = current->next;
  free(current);

  pthread_mutex_unlock(&(list->mutex));
  return data;
}
