/* Copyright (C) 2012 Ben Lewis <benjf5+github@gmail.com>
 * Under GPLv3+, see included gzipped file.
 */

/* Just an example AVL tree implementation which stores ... well, let's put
 * integer keys in there for the sake of things.
 */

#include <stdio.h>
#include <stdlib.h>

typedef struct AVL_elem {
  struct AVL_elem * left, * right;
  int tree_height;
  int key;
} AVL_node;

struct Node_list {
  struct Node_list * previous, * next;
  AVL_node * location;
};

AVL_node * insert (AVL_node *, int);
AVL_node * restructure (AVL_node *);


AVL_node * insert (AVL_node * head, int key) {
  AVL_node * current = head;

  if (!current) {
    current = (AVL_node *) malloc (sizeof (AVL_node));
    current->left = current->right = 0;
    current->tree_height = 1;
    current->key = key;
    return current;
  }
  /* All following code may assume that current is actually extant.
   */
  if (current->key == key) return current;

  if (current->key > key) {
    current->left = insert (current->left, key);
    current->tree_height = (!(current->right)) ? current->left->tree_height + 1
      : (current->left->tree_height > current->right->tree_height) ?
      current->left->tree_height + 1 : current->right->tree_height + 1;
  } else {
    current->right = insert (current->right, key);
    current->tree_height = (!(current->left)) ? current->right->tree_height + 1
      : (current->left->tree_height > current->right->tree_height) ?
      current->left->tree_height + 1 : current->right->tree_height + 1;
  }

  if (current->tree_height > 1) {
    /* In this case, there *may* be an imbalance.
     */
    int left_subtree_height = 0, right_subtree_height = 0, height_difference;
    if (current->left) left_subtree_height = current->left->tree_height;
    if (current->right) right_subtree_height = current->right->tree_height;
    height_difference = left_subtree_height - right_subtree_height;
    if ((height_difference < -1) || (height_difference > 1)) {
      current = restructure (current);
    }
  }
  return current;
}

AVL_node * restructure (AVL_node * head) {
  AVL_node * pivot_top = head, * pivot_parent, * pivot_child;
  if (!(pivot_top->right)) {
    pivot_parent = pivot_top->left;
  } else if (!(pivot_top->left)) {
    pivot_parent = pivot_top->right;
  } else {
    pivot_parent = ((pivot_top->left->tree_height -
                     pivot_top->right->tree_height) < -1) ?
      pivot_top->right : pivot_top->left;
  }
  /* At least now we know where the parent node is; this neatly cleans up the
   * question of "is there just a left node, just a right node, or both?"  Now
   * to do the same for the child node.
   */
  if (!(pivot_parent->right)) {
    pivot_child = pivot_parent->left;
  } else if (!(pivot_parent->left)) {
    pivot_child = pivot_parent->right;
  } else if (pivot_parent->left->tree_height < pivot_parent->right->tree_height) {
    pivot_child = pivot_parent->right;
  } else {
    pivot_child = pivot_parent->left;
  }
  /* The last two are a question of how the rotation will happen—and this holds
   * an important small truth: if the subtrees are balanced, there is no reason
   * for this rotation to occur, so I can ignore the equality case.
   */

  switch (0 + 1*(pivot_top->key < pivot_parent->key)
          + 2 * (pivot_parent->key < pivot_child->key)) {
    case 0:
      pivot_top->left = pivot_parent->right;
      pivot_parent->right = pivot_top;
      return pivot_parent;
      break;
    case 1:
      pivot_top->right = pivot_child->left;
      pivot_parent->left = pivot_child->right;
      pivot_child->left = pivot_top;
      pivot_child->right = pivot_parent;
      return pivot_child;
      break;
    case 2:
      pivot_top->left = pivot_child->right;
      pivot_parent->right = pivot_child->left;
      pivot_child->left = pivot_parent;
      pivot_child->right = pivot_top;
      return pivot_child;
      break;
    case 3:
      pivot_top->right = pivot_parent->left;
      pivot_parent->left = pivot_top;
      return pivot_parent;
      break;
  }

  return head;
  /* This is honestly only here to make the compiler happy, since
   * the four cases shown above are the only four ways for this
   * operation to be executed.
   */
}

int main () {
  AVL_node * root = 0;
  root = insert (root, 5);
  printf ("%d\n", root->key);
  root = insert (root, 7);
  /*  root = insert (root, 3);*/
  root = insert (root, 8);
  printf ("%d\n", root->key);
  return 0;
}
  

