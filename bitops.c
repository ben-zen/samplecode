/* Copyright (C) 2012 Ben Lewis <benjf5+github@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdio.h>

/* Function declarations */
unsigned setbits (unsigned x, int p, int n, unsigned y);
unsigned invert (unsigned x, int p, int n);
unsigned int rightrot (unsigned int x, int n);

/* K&R exercise 2-6 */
/* Write a function that returns x with the n bits that start at p replaced with
 * the n rightmost bits of y, leaving all other bits unchanged.
 */
unsigned setbits (unsigned x, int p, int n, unsigned y) {
  /* This function does not check inputs for correctness, so if you hand it
   * nonsense values it will spit out nonsense values. Caveat Utilitor.
   */
  return (x & ~(~(~0 << n) << (p - n))) | ((~(~0 << n) & y) << (p - n));
}

/* K&R exercise 2-7 */
/* Write a function that returns x with the n bits beginning at the position p
 * inverted, leaving the rest unchanged.
 */
unsigned invert (unsigned x, int p, int n) {
  /* sets n bits right of p to be the inverse of what they were initially. */
  return x ^ ((~(~0 << n)) << (p - n));
}

/* K&R exercise 2-8 */
/* Write a function that returns the value of the integer x rotated by n bit
 * positions.
 */
unsigned int rightrot (unsigned int x, int n) {
  /* Returns x rotated by n bits */
  unsigned int temp = ~(~0 << n) & x;
  temp = temp << ((sizeof(unsigned int) *8) - n);
  return (x >> n) | temp;
}

int main() {
  unsigned char x, y, t, q;
  unsigned int z, r;
  /* x = 0010 1101
   * x = 055
   */
  x = '\x2D';
  /* y = 1101 0001
   * y = 321
   */
  y = '\xD1';
  /* This was before I realised that z has to be an int.
   * z = 1001 0110
   * z = '\x96';
   */
  z = 5;
  
  /* t is what setbits (x, 5, 3, y) should return, ie
   * t = 0010 0101
   * t = 045
   */
  t = '\x25';
  /* q is what invert (y, 4, 4) should return, ie
   * q = 1101 1110
   * q = 336
   */
  q = '\xDE';

  printf ("Expected setbits output: %o\n", t);
  printf ("Actual setbits output: %o\n", setbits (x, 5, 3, y));
  printf ("Expected invert output: %o\n", q);
  printf ("Actual invert output: %o\n", invert (y, 4, 4));
  printf ("z value: %o\n", z);
  printf ("z, rotated by 3: %o\n", rightrot (z, 3));
}
