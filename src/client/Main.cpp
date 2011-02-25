#include <cstdio>
#include <cstring>

#include "Connection.h"
#include "Exceptions.h"

int main()
{
  try
  {
    Connection loop("127.0.0.1", 8982);
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

