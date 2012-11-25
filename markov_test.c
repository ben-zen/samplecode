/* Copyright (C) 2012 Ben Lewis <benjf5+github@gmail.com>
 * Licensed under GPLv3+, see included gzipped file.
 * markov_test.c: Example implementation of the Markov chain in markov.c.
 */

#include <stdio.h>
#include "markov.h"

/* Function declarations */

int comp_char (void *, void *);

/* This is just a comparator function to work with the keys I use in this
 * example, namely a single character.  The trick is to cast each pointer to a
 * character pointer before dereferencing it and storing it in a variable, which
 * I did merely for readability.  The last line uses the internal conversion of
 * boolean operations to integers to return 1 if left is greater than right, -1
 * if left is less than right, and 0 is they're equal.  Incidentally, this
 * identical comparison would work for any numeric type, just replacing char
 * with the type.
 */
int comp_char (void * left_op, void * right_op) {
  char l_val = (* (char *) left_op);
  char r_val = (* (char *) right_op);
  return (l_val > r_val) - (l_val < r_val);
}

int main () {
  /* Create the keys I'll be using. */
  const char cheese = 'C';
  const char lettuce = 'L';
  const char grapes = 'G';
  initialize_markov_chain (comp_char);
  return 0;
}
