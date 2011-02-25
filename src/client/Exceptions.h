#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

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

struct ParseException: Exception
{
  ParseException(const char *text): Exception(text) {}
};

struct SocketException: Exception
{
  SocketException(const char *text): Exception(text);
}

#endif // EXCEPTIONS_H

