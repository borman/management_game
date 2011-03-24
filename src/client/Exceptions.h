#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <cerrno>
#include "StdLib.h"

struct Exception 
{
  Exception(const String &text)
    : text(text) {}

  String text;
};

struct OutOfBoundsException: Exception
{
  OutOfBoundsException(const String &text): Exception(text) {}
};

struct StringFormatException: Exception
{
  StringFormatException(const String &text): Exception(text) {}
};

struct ParserException: Exception
{
  ParserException(const String &text): Exception(text) {}
};

struct SocketException: Exception
{
  SocketException(const String &text): Exception(text), err(errno) {}
  int err;
};

struct CommandException: Exception
{
  CommandException(const String &text): Exception(text) {}
};

struct UnexpectedStateException: Exception
{
  UnexpectedStateException(const String &text): Exception(text) {}
};

#endif // EXCEPTIONS_H

