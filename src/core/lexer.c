/*
 * Copyright 2010, Mikhail "borman" Borisov <borisov.mikhail@gmail.com>
 *
 * This file is part of borman's management game server.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <strings.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "core/lexer.h"

/**
 * Command lexer
 * Splits command line into a list of words
 * Implemented as a finite state machine
 *
 * Tokens are stored internally as a sequence of 0-terminated strings in a 
 * single buffer.
 *
 * Lexing is performed in 2 passes:
 * 1) make a chain of 0-separated strings
 * 2) make a list of strings
 *
 * Implemented syntax features:
 * - enclosing verbatim strings in double quotes
 * - escaping symbols in quoted string and plain text ('\' + symbol) -> symbol
 */ 

typedef enum
{
  ST_WHITESPACE,
  ST_WORD,
  ST_QUOTE,
  ST_ESCAPE,
  ST_QUOTE_ESCAPE,
  ST_ERROR
} LexerState;


static List *make_token_list(Buffer *strings);


/* Lexer FSM main loop */
TokenList *lexer_split(const char *src)
{
  LexerState state = ST_WHITESPACE;
  const size_t length = strlen(src);
  Buffer *tokens = buffer_new();
  size_t i;

  /* terminating zero is handled like a normal character */
  for (i=0; state!=ST_ERROR && i<length+1; i++)
  {
    const char c = src[i];

    switch (state)
    {
      case ST_WHITESPACE:
        if (c=='"') /* start a quoted string, new token */
          state = ST_QUOTE;
        else if (c=='\\') /* start escape-sequence, new token */
          state = ST_ESCAPE;
        else if (c!='\0' && !isspace(c)) /* start a new token */
        {
          state = ST_WORD;
          buffer_putchar(tokens, c);
        }
        /* else: whitespace -> ignore */
        break;

      case ST_QUOTE:
        if (c=='"') /* end a quoted string */
          state = ST_WORD;
        else if (c=='\\') /* start escape-sequence */
          state = ST_QUOTE_ESCAPE;
        else if (c!='\0') /* append a char from inside quotes to token */
          buffer_putchar(tokens, c);
        else /* '\0' -> set error flag */
          state = ST_ERROR;
        break;

      case ST_WORD:
        if (c=='\0' || isspace(c)) /* end a token */
        {
          state = ST_WHITESPACE;
          buffer_putchar(tokens, '\0');
        }
        else if (c=='"') /* start a quoted string */
          state = ST_QUOTE;
        else if (c=='\\') /* start escape-sequence */
          state = ST_ESCAPE; 
        else /* append a char to token */
          buffer_putchar(tokens, c);
        break;

      case ST_ESCAPE:
        if (c!='\0')
        {
          state = ST_WORD;
          buffer_putchar(tokens, c);
        }
        else /* '\0' -> set error flag */
          state = ST_ERROR;
        break;

      case ST_QUOTE_ESCAPE:
        if (c!='\0')
        {
          state = ST_QUOTE;
          buffer_putchar(tokens, c);
        }
        else /* '\0' -> set error flag */
          state = ST_ERROR;
        break;

      default:
        break;
    }    
  }

  /* Detect lexing errors */
  if (state == ST_ERROR)
  {
    buffer_delete(tokens);

    return NULL;
  }
  else 
  {
    TokenList *tl = (TokenList *) malloc(sizeof(TokenList));
    tl->buf = tokens;
    tl->tokens = make_token_list(tokens);
    return tl;
  }
}


void lexer_delete(TokenList *tl)
{
  buffer_delete(tl->buf);
  list_delete(tl->tokens);
  free(tl);
}



/* Make a list out of a chain of strings. */
static List *make_token_list(Buffer *strings)
{
  size_t pos = 0;
  List *list = NULL;

  while (pos<strings->length)
  {
    size_t delta = strlen(strings->c_str+pos);

    list = list_push(list, char *, strings->c_str + pos);
    pos += delta+1;
  }

  return list_reverse(list);
}
