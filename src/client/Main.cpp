extern "C"
{
#include <unistd.h>
}

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "Exceptions.h"
#include "Session.h"
#include "GenericNameGenerator.h"
#include "HumanActor.h"
#include "DumbActor.h"
#include "Term.h"



int main()
{
  // Term::allowColor = isatty(STDOUT_FILENO);

  try
  {
    DumbActor actor;
    GenericNameGenerator namegen;
    //Session session(InetAddress("127.0.0.1", 8982));
    Session session(UnixAddress("/tmp/management-game"));
    session.login(&namegen);
    session.playGame(&actor);
  }
  catch (const SocketException &e)
  {
    printf("Socket exception in %s: %s%s%s\n",
        e.text.c_str(),
        Term::SetRed, strerror(e.err), Term::ResetColor);
  }
  catch (const Exception &e)
  {
    printf("Generic exception: %s%s%s\n",
        Term::SetRed, e.text.c_str(), Term::ResetColor);
  }
  catch (...)
  {
    printf("Alien exception caught\n");
    abort();
  }

  return 0;
}

