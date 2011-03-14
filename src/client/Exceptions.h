#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <cerrno>
#include <string>

struct Exception 
{
  Exception(const std::string &text)
    : text(text) {}

  std::string text;
};

struct OutOfBoundsException: Exception
{
  OutOfBoundsException(const std::string &text): Exception(text) {}
};

struct StringFormatException: Exception
{
  StringFormatException(const std::string &text): Exception(text) {}
};

struct ParserException: Exception
{
  ParserException(const std::string &text): Exception(text) {}
};

struct SocketException: Exception
{
  SocketException(const std::string &text): Exception(text), err(errno) {}
  int err;
};

struct CommandException: Exception
{
  CommandException(const std::string &text): Exception(text) {}
};

#endif // EXCEPTIONS_H

