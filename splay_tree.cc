/* Copyright (C) 2013 Ben Lewis <benjf5@gmail.com>
 * LGPLv3+
 */

#include <exception>

// template <class V>
class Splay_Tree {

private:
  
  class node {
  public:
    node * left, * right;
    int key;
    //    V value;

    node(int k/*, V val*/) {
      left = NULL;
      right = NULL;
      k = key;
      //      value = V(val);
    }
    
  };

  node * root;
  
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

  void remove_nodes (node * nd) {
    if (nd != NULL) {
      if (nd->left != NULL) {
        remove_nodes (nd->left);
      }
      if (nd->right != NULL) {
        remove_nodes (nd->right);
      }
      delete nd;
    }
  }

public:

  // Splay_Tree();
  // V find (int key);
  // bool insert (int key, V value);
  // void remove (int key);

  Splay_Tree/*<V>*/ () {
    root = NULL;
  }

  ~Splay_Tree/*<V>*/ () {
    remove_nodes (root);
  }

  int find (int key) {
    root = splay (key, root);
    if (root != NULL) {
      if (key == root->key) {
        return root->key;
      }
    }
    throw 1; // replace with a *real* exception eventually.
  }

  bool insert (int key/*, V value*/) {
    try {
      node * ins_node = new node(key/*, value*/);
      node * current = root;
      if (current != NULL) {
        bool located = false;
        while (!located) {
          if (key > current->key) {
            if (current->right != NULL) {
              current = current->right;
            } else {
              current->right = ins_node;
              located = true;
            }
          } else if (key < current->key) {
            if (current->left != NULL) {
              current = current->left;
            } else {
              current->left = ins_node;
              located = true;
            }
          } else {
            throw 1; // again, better exception (key already present)
          }
        }
      } else {
        root = ins_node;
      }
    } catch (std::exception & e) {
      return false;
    }
    root = splay (key, root);
    return true;
  }

  void remove (int key) {
    root = splay (key, root);
    if (root != NULL) {
      if (key == root->key) {
        node * left_subtree = root->left;
        node * right_subtree = root->right;
        left_subtree = splay(key, left_subtree);
        // Raises the largest key less than the removed key to the root of the
        // tree.
        /*        delete root->value;*/
        delete root;
        if (left_subtree != NULL) {
          left_subtree->right = right_subtree;
          root = left_subtree;
        } else {
          root = right_subtree;
        }
        // Always place the greatest key less than the deleted key 
      }
    }
    throw 1; // will also replace with a real exception.
  }

};


