/* Copyright (C) 2012 Ben Lewis <benjf5+github@gmail.com>
 * Under GPLv3+, see included gzipped file.
 */

/* Just an example AVL tree implementation which stores ... well, let's put
 * integer keys in there for the sake of things.
 *
 * While using an int to maintain the tree height is not *ideal*, as it offers
 * only 30 levels in which to store data, that provides for 2^31-1
 * nodes. Somehow I'm not particularly worried about that overflowing.
 */

#include <stdio.h>
#include <stdlib.h>

typedef struct AVL_elem {
  struct AVL_elem * left, * right;
  int tree_height;
  int key;
} AVL_node;

AVL_node * insert_node (AVL_node *, int);
AVL_node * restructure (AVL_node *);
AVL_node * remove_node (AVL_node *, int);

/* Add a height computation method, rename tree_height to height, because
 * tree_height is a bad name.  Furthermore, add considerations for when the node
 * is not found in remove_node().
 */

AVL_node * insert_node (AVL_node * head, int key) {
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
    current->left = insert_node (current->left, key);
    current->tree_height = (!(current->right)) ? current->left->tree_height + 1
      : (current->left->tree_height > current->right->tree_height) ?
      current->left->tree_height + 1 : current->right->tree_height + 1;
  } else {
    current->right = insert_node (current->right, key);
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
  int pivot_top_ltree_height, pivot_top_rtree_height, pivot_parent_ltree_height,
    pivot_parent_rtree_height;
  switch (0 + 1*(pivot_top->key < pivot_parent->key)
          + 2 * (pivot_parent->key < pivot_child->key)) {
    case 0:
      pivot_top->left = pivot_parent->right;
      pivot_parent->right = pivot_top;

      pivot_top_ltree_height = (!(pivot_top->left)) ? 0 :
        pivot_top->left->tree_height;
      pivot_top_rtree_height = (!(pivot_top->right)) ? 0 :
        pivot_top->tree_height;
      pivot_top->tree_height = (pivot_top_ltree_height > pivot_top_rtree_height)
        ? pivot_top_ltree_height + 1 : pivot_top_rtree_height + 1;
      pivot_parent->tree_height = (pivot_child->tree_height >
                                   pivot_top->tree_height) ?
        pivot_child->tree_height + 1 : pivot_top->tree_height + 1;
      /* Since the maximal difference between any two tree heights is 2
       * pre-reordering, in this situation, it doesn't matter that this does not
       * check more thoroughly. Furthermore, pivot_child did not have any change
       * to its subtrees.
       */

      return pivot_parent;
      break;
    case 1:
      pivot_top->right = pivot_child->left;
      pivot_parent->left = pivot_child->right;

      /* Now fix the tree heights on each of these. */
      pivot_top_ltree_height = (!(pivot_top->left)) ? 0 :
        pivot_top->left->tree_height;
      pivot_top_rtree_height = (!(pivot_top->right)) ? 0 :
        pivot_top->right->tree_height;
      pivot_top->tree_height = (pivot_top_ltree_height > pivot_top_rtree_height)
        ? pivot_top_ltree_height + 1 : pivot_top_rtree_height + 1;
      pivot_parent_ltree_height = (!(pivot_parent->left)) ? 0 :
        pivot_parent->left->tree_height;
      pivot_parent_rtree_height = (!(pivot_parent->right)) ? 0 :
        pivot_parent->right->tree_height;
      pivot_parent->tree_height = (pivot_parent_ltree_height >
                                   pivot_parent_rtree_height ) ?
        pivot_parent_ltree_height + 1 : pivot_parent_rtree_height + 1;

      pivot_child->left = pivot_top;
      pivot_child->right = pivot_parent;

      pivot_child->tree_height = (pivot_child->left->tree_height >
                                  pivot_child->right->tree_height) ?
        pivot_child->left->tree_height + 1 : pivot_child->right->tree_height +
        1;

      return pivot_child;
      break;
    case 2:
      pivot_top->left = pivot_child->right;
      pivot_parent->right = pivot_child->left;

      /* Now to use the same code as from above to correct the nodes' tree
       * heights.
       */
      pivot_top_ltree_height = (!(pivot_top->left)) ? 0 :
        pivot_top->left->tree_height;
      pivot_top_rtree_height = (!(pivot_top->right)) ? 0 :
        pivot_top->right->tree_height;
      pivot_top->tree_height = (pivot_top_ltree_height > pivot_top_rtree_height)
        ? pivot_top_ltree_height + 1 : pivot_top_rtree_height + 1;
      pivot_parent_ltree_height = (!(pivot_parent->left)) ? 0 :
        pivot_parent->left->tree_height;
      pivot_parent_rtree_height = (!(pivot_parent->right)) ? 0 :
        pivot_parent->right->tree_height;
      pivot_parent->tree_height = (pivot_parent_ltree_height >
                                   pivot_parent_rtree_height ) ?
        pivot_parent_ltree_height + 1 : pivot_parent_rtree_height + 1;

      pivot_child->left = pivot_parent;
      pivot_child->right = pivot_top;

      pivot_child->tree_height = (pivot_child->left->tree_height >
                                  pivot_child->right->tree_height) ?
        pivot_child->left->tree_height + 1 : pivot_child->right->tree_height +
        1;

      return pivot_child;
      break;
    case 3:
      pivot_top->right = pivot_parent->left;
      pivot_parent->left = pivot_top;

      pivot_top_ltree_height = (!(pivot_top->left)) ? 0 :
        pivot_top->left->tree_height;
      pivot_top_rtree_height = (!(pivot_top->right)) ? 0 :
        pivot_top->tree_height;
      pivot_top->tree_height = (pivot_top_ltree_height > pivot_top_rtree_height)
        ? pivot_top_ltree_height + 1 : pivot_top_rtree_height + 1;
      pivot_parent->tree_height = (pivot_child->tree_height >
                                   pivot_top->tree_height) ?
        pivot_child->tree_height + 1 : pivot_top->tree_height + 1;
      /* This is a copy-paste from above, since the conditions on each are
       * identical. 
       */

      return pivot_parent;
      break;
  }

  return head;
  /* This is honestly only here to make the compiler happy, since
   * the four cases shown above are the only four ways for this
   * operation to be executed.
   */
}

AVL_node * remove_node (AVL_node * head, int key) {
  int left_subtree_height, right_subtree_height;
  if (!head) return head;
  if (head->key > key) {
    head->left = remove_node (head->left, key);

    left_subtree_height = (!(head->left)) ? 0 : head->left->tree_height;
    right_subtree_height = (!(head->right)) ? 0 : head->right->tree_height;

    head->tree_height = (left_subtree_height > right_subtree_height) ?
      left_subtree_height + 1 : right_subtree_height + 1;

    return head;
  } else if (head->key < key) {
    head->right = remove_node (head->right, key);

    left_subtree_height = (!(head->left)) ? 0 : head->left->tree_height;
    right_subtree_height = (!(head->right)) ? 0 : head->right->tree_height;

    head->tree_height = (left_subtree_height > right_subtree_height) ?
      left_subtree_height + 1 : right_subtree_height + 1;

    return head;
  } else {
    AVL_node *new_head;
    switch (0 + 1*(!(head->left)) + 2*(!(head->right))) {
      case 0:
        {
          AVL_node * replacement;
          replacement = head->right;
          while (replacement->left) {
            replacement = replacement->left;
          }
          int replacement_key = replacement->key;
          head->right = remove_node (head->right, replacement_key);
          head->key = replacement_key;
          left_subtree_height = (!(head->left)) ? 0 : head->left->tree_height;
          right_subtree_height = (!(head->right)) ? 0 :
            head->right->tree_height; 
          head->tree_height = (left_subtree_height > right_subtree_height) ?
            left_subtree_height + 1 : right_subtree_height + 1;
        }
        return head;        
      case 1:
        new_head = head->right;
        free ((void *) head);
        return new_head;
        break;
      case 2:
        new_head = head->left;
        free ((void *) head);
        return new_head;
        break;
      case 3:
        /* In this case, the node has no children, so return NULL. */
        free ((void *) head);
        return (AVL_node *) 0;
    }
  }
  return (AVL_node *) 0;
}

int main () {
  AVL_node * root = 0;
  root = insert_node (root, 5);
  printf ("(%d, %d)\n", root->key, root->tree_height);

  root = insert_node (root, 7);
  printf ("(%d, %d)\n", root->key, root->tree_height);
  /*  root = insert_node (root, 3);*/
  root = insert_node (root, 8);
  printf ("(%d, %d)\n", root->key, root->tree_height);
  root = remove_node (root, 7);
  printf ("(%d, %d)\n", root->key, root->tree_height);
  return 0;
}
  

