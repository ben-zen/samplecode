/* Copyright (C) 2012 Ben Lewis <benjf5+github@gmail.com>
 * Under GPLv3+, see included gzipped file.
 */

/* Just an example AVL tree implementation which stores ... well, let's put
 * integer keys in there for the sake of things.
 */

struct AVL_node {
  AVL_node * left, right;
  unsigned short int tree_height;
  int key;
};

struct Node_list {
  Node_list * previous, next;
  AVL_node * location;
};

AVL_node * insert (int new_key);
AVL_node * rebalance (AVL_node * current);

/* The key to this is that if abs(left->tree_height - right->tree_height) > 1,
 * pivoting is necessary.  As a matter of fact, if left->tree_height -
 * right->tree_height > 1, then there needs to be a left pivot, and if it's less
 * than -1, there needs to be a left pivot.
 */


/* I'm considering writing this as a recursive function, since the non-recursive
 * function requires an extra data structure to manage the parent, grandparent,
 * etc.
 */

AVL_node * insert (AVL_node * head, int new_key) {
  /* First case to consider is that head is null and we are inserting a new node
   * right there, since that's the first of two stopping conditions.
   */
  if (!head) {
    AVL_node * new_leaf = (AVL_node *) malloc (sizeof (AVL_node));
    new_leaf->left = new_leaf->right = 0;
    new_leaf->key = new_key;
    new_leaf->tree_height = 1;
    return new_leaf;
  }
  /* The second case to consider is that the node has actually been found, in
   * which case no action other than returning the node need be taken, since the
   * tree height doesn't change at any point, or anything.  There's no good way
   * to flag this to avoid calling rebalance, though (although it wouldn't need
   * to do anything.
   */
  if (head->key == new_key) {
    return head;
  }
  /* Finally we get into the other, more complicated situations—which are also
   * more interesting.
   */
  if (head->key > new_key) {
    head->left = insert (head->left, new_key);
  } else {
    head->right = insert (head->right, new_key);
  }
  /* Now there's a situation to consider; we can't know which if either of the
   * children of this node has information in it without testing, so test with a
   * switch statement, values between 0 and 3.
   */
  switch (0 + 1*(!(head->left)) + 2*(!(head->right))) {
    case 0:
      head->tree_height = ((head->left->tree_height > head->right->tree_height)
                           ? head->left->tree_height : head->right->tree_height
                           ) + 1 ;
      break;
    case 1:
      head->tree_height = head->right->tree_height + 1;
      break;
    case 2:
      head->tree_height = head->left->tree_height + 1;
      break;
    case 3:
      head->tree_height = 1;
      break;
  }
  /* Now that the tree height has been safely determined, given that a key has
   * been added (probably; all we know is that it was handed down the chain.)
   * Now it's time to return our current node, since it's filling in for
   * somebody, somewhere.
   */
  return head;
}

AVL_node * rebalance (AVL_node * current) {
  int node_states = 1*(!(current->left)) + 2*(!(current->right));
  if (node_states == 3) return current;
  if (node_states == 2) {
    if (current->left->tree_height == 1) return current;

  }
  if (node_states == 1) {
    if (current->right->tree_height == 1) return current;


  }
  /* Now the only cases which remain are examples in which both left and right
   * sub-trees exist.  The first, and simplest case is that they differ by at
   * most one.
   */
  int height_difference = current->left->tree_height -
    current->right->tree_height;
  if ((height_difference >= -1) && (height_difference <= 1)) return current;
  
}
    
