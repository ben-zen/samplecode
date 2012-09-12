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

/* Exercise 1-24 in ANSI edition of K&R. */

/* Simple syntax checker in C.  Checks for open parentheses, brackets, braces,
 * quotation marks (single or double) and comments.  Does not check for proper
 * use of semicolons or any other language feature, so just because this program
 * returns a positive result for your code does *not* mean it will compile ;)
 */

#include <stdio.h>

#define OPEN_PAREN  '('
#define CLOSE_PAREN ')'
#define OPEN_BRACKET '['
#define CLOSE_BRACKET ']'
#define OPEN_BRACE '{'
#define CLOSE_BRACE '}'
#define SINGLE_QUOT '\''
#define DOUBLE_QUOT '\"'

/* Function declarations */
int enclosure_test ( char opening_symbol, FILE *input );
int comment_test ( FILE *input );
int string_test ( char marker, FILE *input );
char paired_symbol ( char symbol );
int main ( int argc, char *argv[] );

char paired_symbol ( char symbol ) {
  char opposing_symbol;
  switch (symbol) {
    case OPEN_PAREN :
      opposing_symbol = CLOSE_PAREN;
      break;
    case OPEN_BRACKET :
      opposing_symbol = CLOSE_BRACKET;
      break;
    case OPEN_BRACE :
      opposing_symbol = CLOSE_BRACE;
      break;
    case SINGLE_QUOT :
      opposing_symbol = SINGLE_QUOT;
      break;
    case DOUBLE_QUOT :
      opposing_symbol = DOUBLE_QUOT;
      break;
    default :
      opposing_symbol = '\0';
      break;
  }
  return opposing_symbol;
}

int enclosure_test ( char opening_symbol, FILE *input ) {
  int current, comment_check, status = 0;
  char closing_symbol = paired_symbol (opening_symbol);
  
  if ( !closing_symbol ) return 1;
  /* Just a one-line check for a null character in closing_symbol. */
  
  while ( ( current = fgetc ( input ) ) != EOF ) {
    if ( current == closing_symbol ) {
      break;
    }
    switch ( current ) {
      case OPEN_BRACE :
        status = enclosure_test ( OPEN_BRACE, input );
        break;
      case OPEN_BRACKET :
        status = enclosure_test ( OPEN_BRACKET, input );
        break;
      case OPEN_PAREN :
        status = enclosure_test ( OPEN_PAREN, input );
        break;
      case '/' :
        if ( ( comment_check = fgetc ( input ) ) == '*' ) {
          status = comment_test ( input );
        } else {
          if ( comment_check == EOF ) {
            printf ( "End of file reached.\n" );
            return 1;
          } else {
            ungetc ( comment_check , input );
            /* May need a check for EOF here. */
          }
        }
        break;
      case SINGLE_QUOT :
        status = string_test ( SINGLE_QUOT, input );
        break;
      case DOUBLE_QUOT :
        status = string_test ( DOUBLE_QUOT, input );
        break;
    }
    if ( status ) break;
  }

  if ( current == EOF ) status = 1;
  return status;

}

int comment_test ( FILE *input ) {
  int current, comment_next_char, status = 0;
  
  while ( ( current = fgetc ( input ) ) != EOF ) {
    if ( current == '*' ) {
      if ( ( comment_next_char = fgetc ( input ) ) == '/' ) break;
      else {
        ungetc ( comment_next_char , input );
      }
    }
    /* Test if current is * and if following character (without popping) is
     * /. If it is, break and pop following character in order to pop the
     * /. Else read next character.
     */
  }
  if ( current == EOF ) status = 1;
  return status;
}

int string_test ( char marker, FILE *input ) {
  int current;
  while ( ( current = fgetc ( input ) ) != marker ) {
    if ( current == '\\' ) {
      fgetc ( input );
    }
    /* Discarding one character of input as that is an escaped character, so
     * it doesn't matter what it is, even if it is ' or ".
     */
    if ( current == EOF ) {
      break;
    }
  }
  if ( current == EOF ) {
    return 1;
  } else {
    return 0;
  }
}

int main ( int argc, char *argv[] ) {
  int current, comment_check, status = 0;
  if ( argc != 2 ) {
    printf ("syntaxcheck should be called with a file.\n");
    return 2;
  }
  
  FILE *input_file = fopen ( argv[1], "r" );
  if ( input_file == NULL ) {
    printf ( "File was not readable.  Call syntaxcheck with a readable file.\n"
  );
    return 3;
  }
  
  while ( ( current = fgetc ( input_file ) ) != EOF ) {
    switch ( current ) {
      case OPEN_BRACE :
        status = enclosure_test ( OPEN_BRACE, input_file );
        break;
      case OPEN_BRACKET :
        status = enclosure_test ( OPEN_BRACKET, input_file );
        break;
      case OPEN_PAREN :
        status = enclosure_test ( OPEN_PAREN, input_file );
        break;
      case '/' :
        if ( ( comment_check = fgetc ( input_file ) ) == '*' ) {
          status = comment_test ( input_file );
        } else {
          if ( comment_check == EOF ) {
            printf ( "End of file reached.\n" );
            return 0;
          } else {
            ungetc ( comment_check , input_file );
            /* May need a check for EOF here. */
          }
        }
        break;
      case SINGLE_QUOT :
        status = string_test ( SINGLE_QUOT, input_file );
        break;
      case DOUBLE_QUOT :
        status = string_test ( DOUBLE_QUOT, input_file );
        break;
    }
    if ( status ) break;
  }
  
  if ( status == 1 ) {
    printf ( "Syntax error in code.  Check for misplaced brackets, braces, "
             "parentheses, a missing quote mark, or an unbounded comment.\n" );
  }
  /* Other status levels will be posted here as well.  It is theoretically
   * possible to determine what bracket/brace/paren/quote mark is left out from
   * a status number, if I wanted to set that up.
   */
  return status;
}
