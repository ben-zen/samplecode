/* Copyright (C) 2012 Ben Lewis <benjf5+github@gmail.com>
 *
 * GPLv3
 */

/* K&R exercise 2-6 */

unsigned setbits (unsigned x, int p, int n, unsigned y) {
  unsigned temp = ~ ( ( ~0 << n ) << p );
  unsigned ret_temp = x & temp;
  temp = (~ temp) & ( y << p );
  return ret_temp | temp;
}
/* I *think* this works. It's kinda wonky, though. */

/* K&R exercise 2-7 */

unsigned invert (unsigned x, int p, int n) {
  /* sets n bits right of p to be the inverse of what they were initially. */
  return ((x & ~((~0 << n) << (p + 1 - n))) |
	  (~((x & (~0 << n) << (p + 1 - n)) >> (p + 1 - n))) << (p - n));
  /* Again, not so sure this is correct. */
}

/* K&R exercise 2-8 */
unsigned int rightrot (unsigned int x, int n) {
  /* Returned x rotated by n bits */
  unsigned int temp = ~(~0 << n) & x;
  temp = temp << ((sizeof(unsigned int) *8) - n);
  return (x >> n) | temp;
}

int main() {
  unsigned int x, y, z, t, q;
  
}
