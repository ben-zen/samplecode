/* Copyright (C) 2012 Ben Lewis <benjf5+github@gmail.com>
 * Under GPLv3+, see included gzipped file.
 */

#include <stdlib.h>

typedef Colour enum { BLACK, RED };

typedef struct RB_elem {
  struct RB_elem * left, * right;
  int 
