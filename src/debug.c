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


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <time.h>
#include <sys/time.h>

#include "debug.h"

static void log(const char *prefix, const char *format, va_list args);


#define DEBUG_FUNC(name, prefix, color) \
  void name(const char *format, ...) \
  { \
    va_list args; \
    va_start(args, format); \
    log(color prefix, format, args); \
    va_end(args); \
  }  

DEBUG_FUNC(trace,   "[ Trace ]", TERM_FG_CYAN)
DEBUG_FUNC(message, "[Message]", TERM_FG_WHITE)
DEBUG_FUNC(warning, "[Warning]", TERM_FG_RED)

void fatal(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  log(TERM_FG_RED "[ FATAL ]", format, args);
  va_end(args);

  fflush(stdout);
  fflush(stderr);
  abort();
}

static void log(const char *prefix, const char *format, va_list args)
{
#ifdef USE_LOG_TIME_MARKERS
  struct timeval tv;
  struct tm *tm;

  gettimeofday(&tv, NULL);
  tm = localtime(&tv.tv_sec);

  fprintf(stderr, TERM_BOLD TERM_FG_BLACK "%02d:%02d:%02d.%06ld %s " TERM_NORMAL,
     tm->tm_hour, tm->tm_min, tm->tm_sec, tv.tv_usec, prefix);
#else
  fprintf(stderr, TERM_BOLD "%s " TERM_NORMAL, prefix);
#endif
  vfprintf(stderr, format, args);
  fprintf(stderr, "\n");
}

