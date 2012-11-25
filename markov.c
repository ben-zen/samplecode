/* Copyright (C) 2012 Ben Lewis <benjf5+github@gmail.com>
 * A general implementation of a Markov chain with decorated edges.
 */

#include <stdlib.h>
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
  struct markov_elem * left, * right;
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
int (* compare) (void *, void *);

/* Private function prototypes */
int find_markov_node (void * content);

markov_node * splay_markov (void * content, markov_node * head); 
int insert_edge (double dec, edge_node ** head, markov_node * src,
                         markov_node * dest);

/* Functions */

void initialize_markov_chain (int (* comp) (void *, void *)) {
  /* Define compare() as comp() */
  compare = comp;
  /* Initialize random number generator */
  srand (time (NULL));
}

int insert_edge (double dec, edge_node ** head, markov_node * src,
                         markov_node * dest) {
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


markov_node * splay_markov (void * content, markov_node * head) {
  /* This is splay code that relies on (second arg - first arg) (as a general
   * idea.) That is, second arg greater than first arg results in positive
   * value, second arg less than first arg results in negative value, identical
   * arguments results in 0.
   */
  if (head) {
    if (!((* compare) (head->content, content))) {
      return head;
    } else if (((* compare) (head->content, content)) > 0) {
      if (!(head->right)) {
        return head;
      } else {
        head->right = splay_markov (content, head->right);
        markov_node * temp = head->right;
        head->right = temp->left;
        temp->left = head;
        return temp;
      }
    } else { /* In this case, it's less (to the left). */
      if (!(head->left)) {
        return head;
      } else {
        head->left = splay_markov (content, head->left);
        markov_node * temp = head->left;
        head->left = temp->right;
        temp->right = head;
        return temp;
      }
    }
  } /* Else, head was null. */
  return head;
}

int find_markov_node (void * content) {
  /* This function is far more for determining a node's presence than for
  actually returning the location, as each call  */
  markov_head = splay_markov (content, markov_head);
  if (!((* compare) (content, markov_head->content))) {
    return EXIT_SUCCESS;
  } /* else */
  /* The new head is *not* the value sought, so it is not in the graph. */
  return EXIT_FAILURE;
}

int create_markov_node (void * content) {
  /* First, splay to determine if the node already exists. */
  if (find_markov_node (content) == EXIT_FAILURE) {
    markov_node * n_mk_node = (markov_node *) malloc (sizeof (markov_node));
    /* The pointer needs to be assigned for this to happen. */
    if (n_mk_node) {
      n_mk_node->content = content;
      /* Now, at this stage both left and right children will be null, and it
       * has no edges.
       */
      n_mk_node->left = NULL;
      n_mk_node->right = NULL;
      n_mk_node->outbound_edge_list = NULL;
      /* Now we need to position it in the tree.  Which requires traversing the
       * tree as it exists currently.
       */
      markov_node * current = markov_head;
      if (!current) {
        markov_head = n_mk_node;
      } else {
        /* In this case, we need to find the proper node to insert below, and then
         * splay.
         */
        while ((current->left) || (current->right)) {
          if ((* compare) (content, current->content) > 0) {
            /* Should be to the right */
            if (current->right) {
              current = current->right;
              /* Advance to the right in the tree. */
            } else {
              /* Otherwise, we can insert the node here and break out of the
               * loop.
               */
              current->right = n_mk_node;
              break;
            }
          } else {
            /* There is only one "else" case (to the left), since if the node is
             * ever equal, then the initial "find" operation would have
             * succeeded.
             */
            if (current->left) {
              current = current->left;
              /* Advance down and left. */
            } else {
              /* Otherwise, insert and break! */
              current->left = n_mk_node;
              break;
            }
          }
        }
        markov_head = splay_markov (content, markov_head);
      }
      return EXIT_SUCCESS;
    }
  }
  return EXIT_FAILURE;
}

void * find_next_state (void) {
  /* Relies on the initial state having previously been determined. */
  double prob = (double) rand() / ((double) RAND_MAX);
  if (prob <= markov_head->outbound_total) {
    edge_node * current_edge = markov_head->outbound_edge_list;
    while (current_edge) {
      if (prob <= current_edge->decoration) {
        /* As the edges are sorted in maximum to minimum order. */
        markov_head = splay_markov (current_edge->destination->content,
                                    markov_head);
        return markov_head->content;
      } else {
        prob -= current_edge->decoration;
        current_edge = current_edge->next;
      }
    }
  } /* else */
  /* In this case, the probability fell in the range of the implied edge
   * pointing back to the node itself; there can also be an explicit node for that,
   * as well.
   */
  return markov_head->content;
}

int set_initial_state (void * content) {
  if (markov_head) {
    markov_head = splay_markov (content, markov_head);
    if (!((* compare) (content, markov_head->content))) {
      return EXIT_SUCCESS;
    }
  }
  /* Otherwise the initial state was not set, as requested, so return
   * failure.
   */
  return EXIT_FAILURE;
}
