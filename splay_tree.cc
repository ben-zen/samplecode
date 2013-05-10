/* Copyright (C) 2013 Ben Lewis <benjf5@gmail.com>
 * LGPLv3+
 */

template <class V>
class Splay_Tree {

  struct node {
    node * left, right;
    int key;
    V * value;
  };

  
  node root;
  
  node * splay (int key, node * nd) {
    if (nd != NULL) {
      if (key == nd->key) {
        return nd;
      } else if (key > nd->key) {
        node * new_right = splay (key, nd->right);
        if (new_right != NULL) {
          nd->right = new_right->left;
          new_right->left = nd;
          return new_right;
        } // otherwise, jump to the end and return the input node.
      } else {
        node * new_left = splay (key, nd->left);
        if (new_left != NULL) {
          nd->left = new_left->right;
          new_left->right = nd;
          return new_left;
        } // same failure case.
      }
    }
    return nd;
  }

  // public

  V find (int key) {
    root = splay (key, root);
    if (root != NULL) {
      if (key == root->key) {
        return *(root->value);
      }
    } else {
      throw 1; // replace with a *real* exception eventually.
    }
  }

  void remove (int key) {
    root = splay (key, root);
    if (root != NULL) {
      if (key == root->key) {
        node * left_subtree = root->left;
        node * right_subtree = root->right;
        left_subtree = splay(key, left_subtree);
    //     // Raises the largest key less than the removed key to the root of the
    //     // tree.
    //     if (left_subtree != NULL) {
    //       left_subtree->right = right_subtree;
    //       root = left_subtree;
    //     } else {
    //       root = right_subtree;
    //     }
    //   }
    // }
    // throw 1; // will also replace with a real exception.
  }
