/* Copyright (C) 2012 Ben Lewis <benjf5+github@gmail.com>
 * Under GPLv3+, see included gzipped file.
 * markov.h: Header file for markov.c
 * A general implementation of a Markov chain with decorated edges.
 */

extern int create_markov_node (void * content, int (*comp) (void *, void *));
extern int add_edge (void * src, void * dest, double dec, int (*comp) (void *,
                                                                       void*));
extern void * find_next_state (int (*comp) (void *, void *));
