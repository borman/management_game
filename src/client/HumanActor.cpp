#include <cassert>
#include <iostream>
#include <iomanip>
#include "Exceptions.h"
#include "Stanza.h"
#include "HumanActor.h"
#include "Session.h"
#include "Term.h"

using namespace std;

void HumanActor::onGameStart(Session *session)
{
  assert(session != NULL);
}

void HumanActor::onTurn(Session *session)
{
  assert(session != NULL);

  cout << session->gameInfo().market();

  bool turn_must_go_on = true;
  while (turn_must_go_on)
  {
    cout << Term::Bold("Your turn> ");
    string s;
    getline(cin, s);
    try
    {
      Stanza st = Stanza::parse(s);
      if (st.size() == 0)
        continue;
      if (st[0] == "ready")
        turn_must_go_on = false;
      else
        session->execCommand(st);
    }
    catch (const ParserException &e)
    {
      cout << Term::Red("Command syntax error") << endl;
      continue;
    }
    catch (const CommandException &e)
    {
      cout << Term::Red("Command failed: ") << e.text << endl;
      continue;
    }
  }
}

