#include <cstdio>
#include <cstring>

#include "SocketEventLoop.h"
#include "Exceptions.h"

int main()
{
  try
  {
    SocketEventLoop loop("127.0.0.1", 8982);
    loop.run();
  }
  catch (SocketException e)
  {
    printf("Socket exception in %s: %s\n", e.text, strerror(e.err));
  }
  catch (Exception e)
  {
    printf("Generic exception: %s\n", e.text);
  }

  return 0;
}

