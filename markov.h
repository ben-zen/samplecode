/* Copyright (C) 2012 Ben Lewis <benjf5+github@gmail.com>
 * Under GPLv3+, see included gzipped file.
 * markov.h: Header file for markov.c
 * A general implementation of a Markov chain with decorated edges.
 */
#ifndef MARKOV_HEADER
#define MARKOV_HEADER
extern int create_markov_node (void * content);
extern int add_edge (void * src, void * dest, double dec);
extern void * find_next_state (void);
extern int set_initial_state (void * content);
extern void initialize_markov_chain (int (* comp) (void *, void *));
#endif
