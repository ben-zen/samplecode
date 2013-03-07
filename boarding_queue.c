/* Copyright (C) 2013 Ben Lewis
 * LGPL v3+
 */

/* Resulting from considering the nontrivial task of merging queues without
 * pissing people off while waiting in the airport.
 *
 * Starting with a naive approach, then expanding into more concrete forms.
 */

#include <stdlib.h>

typedef struct queue_node {
  struct queue_node * next;
  unsigned int long key;
} q_node_t;

typedef struct delay_queue_wrapper {
  q_node_t * head;
  q_node_t * tail;
  int delay;
  size_t size;
} delay_queue;

unsigned int long select_from_queues (delay_queue * priority, delay_queue* left,
                                      delay_queue * right);
void add_key_to_queue (unsigned int long new_key, delay_queue* queue);
unsigned int long peek_head_key (delay_queue * queue);
unsigned int long pop_head_key (delay_queue * queue);

delay_queue construct_delay_queue (void) {
  delay_queue new_queue;
  new_queue.head = NULL;
  new_queue.tail = NULL;
  new_queue.delay = 0;
  new_queue.size = 0;
  return new_queue;
}

void add_key_to_queue (unsigned int long new_key, delay_queue* queue) {
  q_node_t * new_elem = (q_node_t *) malloc (sizeof (q_node_t));
  if (new_elem) {
    new_elem->next = NULL;
    new_elem->key = new_key;
    if (queue->head) {
      queue->tail->next = new_elem;
      queue->tail = new_elem;
      queue->size++;
    } else {
      /* In this case there is no head, so there is no tail. */
      queue->head = new_elem;
      queue->tail = new_elem;
      queue->size = 1;
    }
  }
  /* Currently this fails silently which is not optimal. */
}
  
unsigned int long peek_head_key (delay_queue * queue) {
  if (queue->head) {
    return queue->head->key;
  }
  return 0;
}

unsigned int long pop_head_key (delay_queue * queue) {
  unsigned int long k = 0;
  if (queue->head) {
    k = queue->head->key;
    q_node_t * to_remove = queue->head;
    queue->head = queue->head->next;
    free (to_remove);
    queue->size--;
    if (!(queue->head)) {
      queue->tail = NULL;
    }
  }
  queue->delay = 0; /* This can later be moved to another function which keys
  delays to how long since they've been satisfied. */
  return k;
}

int get_size (delay_queue * queue) {
  return queue->size;
}

int get_delay (delay_queue * queue) {
  return queue->delay;
}

void increment_delay (delay_queue * queue) {
  if (queue->head) queue->delay++;
}

unsigned int long select_from_queues (delay_queue * priority, delay_queue* left,
                                      delay_queue * right) {
  unsigned int long id = 0; /* This leads to the obvious note that no real ID
                                      should be 0. */
  if (peek_head_key (priority)) {
    id = pop_head_key (priority);
  } else if (peek_head_key (left)) {
    if (peek_head_key (right)) {
      if (get_delay (left) >= get_delay (right)) {
        id = pop_head_key (left);
        increment_delay (right);
      } else {
        id = pop_head_key (right);
        increment_delay (left);
      }
    } else {
      id = pop_head_key (left);
    }
  } else {
    if (peek_head_key (right)) {
      id = pop_head_key (right);
    }
  }
  /* Note two things: this neither updates wait times nor really tracks other
   * state changes.  However, it doesn't need an extra final case as no left, no
   * right is already accounted for.
   */
  return id;
}

/* Changes to anticipate: renaming functions to avoid mention of popping. Also
   currently has no access to tail for adding to queue.
 */
int main (void) {
}
