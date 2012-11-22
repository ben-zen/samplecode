/* Copyright (C) 2012 Ben Lewis <benjf5+github@gmail.com>
 * A general implementation of a Markov chain with decorated edges.
 */

#include <stdlib.h>

/* Two main data structures; every graph node has a list of edges, and each edge
 * node maintains a pointer to each of the graph nodes it is connected to.  In
 * this case, the edge nodes only maintain a single decoration, the probability
 * of a change (as a value between 0 and 1, stored in a float.)
 */

/* Each graph node contains a void pointer for the information it is supposed to
 * be storing, and a pointer to the first edge out of it.
 */

/* A list of nodes is maintained as well, separate of the others, in order to be
 * able to construct the full graph.
 */

typedef struct edge_elem edge_node;
typedef struct markov_elem markov_node;


struct markov_elem {
  struct markov_elem * left, * right;
  struct splay_node * outbound_edge_list;
  double outbound_total;
  void * content;
};

struct edge_elem {
  struct edge_elem * left, * right;
  markov_node * source, * destination;
  double decoration;
};

/* Instantiating the markov chain */
markov_node * markov_head = NULL;

markov_node * find_markov_node (void * content);

markov_node * splay_markov (void * content, markov_node * head,
                           int (* comp) (void *, void *)); 
edge_node * splay_edge (double dec, edge_node * head);

markov_node * splay_markov (void * content, markov_node * head,
                           int (* comp) (void *, void *)) {
  /* This is splay code that relies on (second arg - first arg) (as a general
   * idea.) That is, second arg greater than first arg results in positive
   * value, second arg less than first arg results in negative value, identical
   * arguments results in 0.
   */
  if (head) {
    if (!(comp (head->content, content))) {
      return head;
    } else if ((comp (head->content, content)) > 0) {
      if (!(head->right)) {
        return head;
      } else {
        head->right = splay_markov (content, head->right, comp);
        markov_node * temp = head->right;
        head->right = temp->left;
        temp->left = head;
        return temp;
      }
    } else { /* In this case, it's less (to the left). */
      if (!(head->left)) {
        return head;
      } else {
        head->left = splay_markov (content, head->left, comp);
        markov_node * temp = head->left;
        head->left = temp->right;
        temp->right = head;
        return temp;
      }
    }
  } /* Else, head was null. */
  return head;
}

int create_markov_node (void * content, int (* comp) (void *, void *)) {
  markov_node * current = markov_head;
  while (current)
  if (!markov_head) {
    markov_head = (markov_node *) malloc (sizeof (markov_node));
  }
  /* At this point, if there isn't a node, something went wrong. */
  if (markov_head) {
    markov_node * n_mk_node = (markov_node *) malloc (sizeof (markov_node));
    /* The pointer needs to be assigned for this to happen. */
    if (n_mk_node) {
      n_mk_node->content = content;
      
  
}

int add_edge (markov_node * src, markov_node * dest, double dec) {
  /* Test dec to make sure dec <= 1; if it's greater, this edge is not possible
   * to begin with.
   */
  if (dec <= 1) {
    edge * new_edge = (edge *) malloc (sizeof (edge));
    /* Need to add something to handle an OOM. */
    new_edge->next = NULL
      new_edge->decoration = dec;
    new_edge->source = src;
    new_edge->destination = dest;
    /* Now, add the edge to the source's edge list. */
    if (src->outbound_edge_list) {
      edge * current = src->outbound_edge_list;
      double prob_sum = 0;
      while (current->next) {
        prob_sum += current->decoration;
        current = current->next;
      }
      /* At this point, it has reached the end of the list of edges. */
      prob_sum += dec;
      /* Test for acceptable range   */
      if (prob_sum <= 1) {
        current->next = new_edge;
      } else {
        /* Return a failure state: bad range. */
      }
    } else {  
      /* Otherwise, there was no list to begin with. */
      src->outbound_edge_list = new_edge;
    }
  }
  /* else */
  /* Return a failure state: bad range. */
}


