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

#import <stdio.h>

#define OPEN_PAREN  '('
#define CLOSE_PAREN ')'
#define OPEN_BRACKET '['
#define CLOSE_BRACKET ']'
#define OPEN_BRACE '{'
#define CLOSE_BRACE '}'
#define SINGLE_QUOT '\''
#define DOUBLE_QUOT '\"'

/* So far, my method of attack for this is to consider comments independently of
 * other enclosures, since you can't use a single char to represent to capture
 * its opening or closing marks; as such, there will be a recursive function to
 * determine if a set of enclosing symbols is unbounded or no; if it is properly
 * bounded, it returns 0, but if it is not, it returns 1. This can work quite
 * simply by having a state in each level of the recursion; you can have it as
 * an integer state set initially as 0, and if at any point an EOF is reached
 * *outside* of the main function, set the failure_state at 1 and break the
 * while() loop, thus causing a return of a failing condition; OR this together
 * with the calling function's failure_state, and voilà it's a test. I don't
 * particularly feel like writing it to also handle returning details about the
 * error, but it'd definitely be possible. I feel like I'd need to have it
 * return a struct in that case, but I can rewrite it to be more complete later.
 *
 * If this were C++, I'd have it construct a string as an exception and throw
 * it.
 */

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

  /* There is, of course, a problem with this, which is that I need to be sure
   * not to call it with anything which is not a defined required symbol. That
   * doesn't strike me as too much of a problem, I don't think.
   */

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
      
    /* Check character to see if an opening symbol of another enclosure.  If
     * not, but it is a /, check if character after is * _without_ popping from
     * stack. If it is, pop said character from input and proceed to
     * comment_test.  If it is one or the other of these, and it returns a 1, halt
     * testing and set status to 1, break out of while loop.  Otherwise, read
     * next character.
     */

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
