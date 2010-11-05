/*
 * Copyright 2010, Mikhail "borman" Borisov <borisov.mikhail@gmail.com>
 *
 * This file is part of borman's model management game server.
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


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "debug.h"

#define DEBUG_FUNC(name, prefix, color) \
  void name(const char *format, ...) \
  { \
    va_list args; \
    va_start(args, format); \
    fprintf(stderr, color TERM_BOLD prefix TERM_NORMAL ); \
    vfprintf(stderr, format, args); \
    fprintf(stderr, TERM_NORMAL "\n"); \
    va_end(args); \
  }  

DEBUG_FUNC(trace,   "[ Trace ] ", TERM_FG_CYAN)
DEBUG_FUNC(message, "[Message] ", TERM_FG_WHITE)
DEBUG_FUNC(warning, "[Warning] ", TERM_FG_RED)

void fatal(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  fprintf(stderr, TERM_FG_RED TERM_BOLD "[ FATAL ] " TERM_NORMAL );
  vfprintf(stderr, format, args);
  fprintf(stderr, TERM_NORMAL "\n");
  va_end(args);

  exit(1);
}
