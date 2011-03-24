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
#include "BotActor.h"
#include "Term.h"

using namespace std;

int main()
{
  cin.exceptions(ios_base::failbit | ios_base::eofbit);

  Term::allowColor = isatty(STDOUT_FILENO);

  try
  {
    BotActor actor;
    GenericNameGenerator namegen;
    //Session session(InetAddress("127.0.0.1", 8982));
    Session session(UnixAddress("/tmp/management-game"));
    session.login(&namegen);
    session.playGame(&actor);
  }
  catch (const SocketException &e)
  {
    cout << "Socket exception in " << e.text << ": " << Term::Red(strerror(e.err)) << endl;
  }
  catch (const Exception &e)
  {
    cout << "Generic exception: " << Term::Red(e.text) << endl;
  }
  catch (const ios_base::failure &e)
  {
    cout << "Input failed" << endl;
    exit(1);
  }
  catch (...)
  {
    printf("Alien exception caught\n");
    abort();
  }

  return 0;
}

