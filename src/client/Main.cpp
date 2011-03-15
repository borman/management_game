#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>

#include "Exceptions.h"
#include "Session.h"
#include "GenericNameGenerator.h"
#include "HumanActor.h"

using namespace std;

int main()
{
  cin.exceptions(ios_base::failbit | ios_base::eofbit);

  try
  {
    HumanActor actor;
    GenericNameGenerator namegen;
    //Session session(InetAddress("127.0.0.1", 8982));
    Session session(UnixAddress("/tmp/management-game"));
    session.login(&namegen);
    session.playGame(&actor);
  }
  catch (const SocketException &e)
  {
    cout << "Socket exception in " << e.text << ": " << strerror(e.err) << endl;
  }
  catch (const Exception &e)
  {
    cout << "Generic exception: " << e.text << endl;
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

