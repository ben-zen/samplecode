/* Fizz Buzz in C */
/* Copyright (C) 2012 Benjamin Lewis
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

/* Function declaration */
int fizz_buzz ( int cap );

int main () {
  /* At some point, may replace the fixed value of 100 with a call and possible
   * default value. In fact, I'd probably have a default variable.
   */
  fizz_buzz ( 100 );
  return 0;
}

int fizz_buzz ( int cap ) {
  int i;
  for ( i = 1 ; i <= cap ; i++ ) {
    switch ( 0 + 1 * ( ! ( i % 3 ) ) + 2 * ( ! ( i % 5 ) ) ) {
      case 0 :
	printf ("%d\n",i);
	break;
      case 1 :
	printf ("Fizz\n");
	break;
      case 2 :
	printf ("Buzz\n");
	break;
      case 3 :
        printf ("Fizz Buzz\n");
        break;
    }
  }
  return 0;
}
