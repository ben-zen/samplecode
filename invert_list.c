/* Copyright (C) 2012 Ben Lewis
 *
 *
 */

/* Inverts a singly-linked list.  Expects a struct single_linked_list and that it
 * has a single_linked_list * next in it.  Also expects that the last element
 * has node->next = 0.
 */

struct single_linked_list {
  single_linked_list * next;
  /* some sort of content */
  void * content;
};

single_linked_list * invert (single_linked_list *);

single_linked_list * invert (single_linked_list * lst) {
  single_linked_list * head,current,nxt,prev;
  prev = lst;
  current = lst->next;
  head = nxt = lst->next->next;
  prev->next = 0;
  current->next = prev;
  while (nxt->next) {
    prev = current;
    current = nxt;
    nxt = nxt->next;
    current->next = prev;
  }
  head = nxt;
  return head;
}
