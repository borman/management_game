#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <cerrno>

struct Exception 
{
  Exception(const char *text)
    : text(text) {}

  const char *text;
};

struct OutOfBoundsException: Exception
{
  OutOfBoundsException(const char *text): Exception(text) {}
};

struct StringFormatException: Exception
{
  StringFormatException(const char *text): Exception(text) {}
};

struct ParserException: Exception
{
  ParserException(const char *text): Exception(text) {}
};

struct SocketException: Exception
{
  SocketException(const char *text): Exception(text), err(errno) {}
  int err;
};

#endif // EXCEPTIONS_H

