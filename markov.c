/* Copyright (C) 2012 Ben Lewis <benjf5+github@gmail.com>
 * A general implementation of a Markov chain with decorated edges.
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/* Two main data structures; every graph node has a list of edges, and each edge
 * node maintains a pointer to each of the graph nodes it is connected to.  In
 * this case, the edge nodes only maintain a single decoration, the probability
 * of a change (as a value between 0 and 1, stored in a float.)
 */

/* Each graph node contains a void pointer for the information it is supposed to
 * be storing, and a pointer to the first edge out of it.
 */

/* A tree of nodes is maintained as well, separate of the others, in order to be
 * able to construct the full graph.  This is implemented as a splay tree, to
 * take advantage of its search abilities.
 */

typedef struct edge_elem edge_node;
typedef struct markov_elem markov_node;


struct markov_elem {
  struct markov_elem * next;
  struct edge_elem * outbound_edge_list;
  double outbound_total;
  void * content;
};

struct edge_elem {
  struct edge_elem * next;
  markov_node * destination;
  double decoration;
};

/* Instantiation of markov_head and compare(). */
markov_node * markov_head = NULL;
markov_node * current_state = NULL;
int (* compare) (void *, void *);

/* Private function prototypes */
markov_node * find_markov_node (void * content);

int insert_edge (double dec, edge_node ** head, markov_node * dest);

/* Functions */

void initialize_markov_chain (int (* comp) (void *, void *)) {
  /* Define compare() as comp() */
  compare = comp;
  /* Initialize random number generator */
  srand (time (NULL));
}

int insert_edge (double dec, edge_node ** head, markov_node * dest) {
  if (*head) {
    edge_node * current = (*head);
    while (current->next) {
      if (dec < current->decoration) {
        current = current->next;
      } else {
        break;
      }
    }
    edge_node * temp = (edge_node *) malloc (sizeof (edge_node));
    /* Just need to check and make sure that there wasn't a problem
     * instantiating the new node.
     */
    if (!temp) {
      return EXIT_FAILURE; /* Need to pick a better error. */
    }
    temp->decoration = dec;
    temp->destination = dest;
    if (current->next) {
      temp->next = current->next;
    } else {
      temp->next = NULL;
    }
    current->next = temp;
  } else { /* Otherwise, head was null and we need to insert a node.  This is
              simplified by the pointer to a pointer. */
    edge_node * temp = (edge_node *) malloc (sizeof (edge_node));
    if (!temp) {
      return EXIT_FAILURE;
    }
    temp->decoration = dec;
    temp->destination = dest;
    temp->next = NULL;
    (*head) = temp;
  }
  /*  */
  return EXIT_SUCCESS;
}

markov_node * find_markov_node (void * content) {
  markov_node * temp = markov_head;
  while (temp) {
    if (((* compare) (content, temp->content)) == 0) {
      return temp;
    }
  }
  return NULL;
}

int create_markov_node (void * content) {
  if (!(find_markov_node (content))) {
    markov_node * n_mk_node = (markov_node *) malloc (sizeof (markov_node));
    /* The pointer needs to be assigned for this to happen. */
    if (n_mk_node) {
      n_mk_node->content = content;
      n_mk_node->next = NULL;
      n_mk_node->outbound_edge_list = NULL;
      n_mk_node->outbound_total = 0;
      markov_node * current = markov_head;
      if (!current) {
        markov_head = n_mk_node;
      } else {
        while (current->next) {
          current = current->next;
        }
        current->next = n_mk_node;
      }
      printf ("%c\n", (* ((char *) markov_head->content)));
      return EXIT_SUCCESS;
    }
  }
  return EXIT_FAILURE;
}

void * find_next_state (void) {
  /* Relies on the initial state having previously been determined. */
  double prob = ((double) rand()) / ((double) RAND_MAX);
  printf ("%f3\n",prob);
  edge_node * current_edge = current_state->outbound_edge_list;
  while (current_edge) {
    if (prob <= current_edge->decoration) {
      /* As the edges are sorted in maximum to minimum order. */
      current_state = current_edge->destination;
      return current_state->content;
    } else {
      prob -= current_edge->decoration;
      current_edge = current_edge->next;
    }
  } /* else */
  /* In this case, the probability fell in the range of the implied edge
   * pointing back to the node itself; there can also be an explicit node for that,
   * as well.
   */
  return current_state->content;
}

int add_edge (void * src, void * dest, double dec) {
  if (dec <= 1) {
    markov_node * src_node = find_markov_node (src);
    markov_node * dest_node = find_markov_node (dest);
    if (src_node && dest_node) {
      if ((src_node->outbound_total + dec) <= 1) {
        src_node->outbound_total += dec;
        return insert_edge (dec, &(src_node->outbound_edge_list), dest_node);
      }
    } 
  } /* Otherwise it failed. */
  return EXIT_FAILURE;
}

int set_initial_state (void * content) {
  if (markov_head) {
    current_state = find_markov_node (content);
    if (current_state) {
      return EXIT_SUCCESS;
    }
  }
  /* Otherwise the initial state was not set, as requested, so return
   * failure.
   */
  return EXIT_FAILURE;
}
