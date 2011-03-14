#include <cstdio>
#include <cstring>

#include "Exceptions.h"
#include "Session.h"
#include "GenericNameGenerator.h"

int main()
{
  HumanActor *actor = new HumanActor();
  GenericNameGenerator *namegen = new GenericNameGenerator();
  try
  {
    Session session("127.0.0.1", 8982);
    session.login(namegen);
    while (true)
      session.playGame(actor);
  }
  catch (SocketException e)
  {
    printf("Socket exception in %s: %s\n", e.text.c_str(), strerror(e.err));
  }
  catch (Exception e)
  {
    printf("Generic exception: %s\n", e.text.c_str());
  }

  return 0;
}

