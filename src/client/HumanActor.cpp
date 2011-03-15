#include <cassert>
#include <iostream>
#include <iomanip>
#include "Exceptions.h"
#include "Stanza.h"
#include "HumanActor.h"
#include "Session.h"

using namespace std;

void HumanActor::onGameStart(Session *session)
{
  assert(session != NULL);
}

void HumanActor::onTurn(Session *session)
{
  assert(session != NULL);

  session->gameInfo().market().prettyPrint();

  bool turn_must_go_on = true;
  while (turn_must_go_on)
  {
    cout << "Your turn> ";
    string s;
    getline(cin, s);
    try
    {
      Stanza st = Stanza::parse(s);
      if (st.size() == 0)
        continue;
      session->execCommand(st);
      if (st[0] == "ready")
        turn_must_go_on = false;
    }
    catch (const ParserException &e)
    {
      cout << "Command syntax error" << endl;
      continue;
    }
    catch (const CommandException &e)
    {
      cout << "Command failed: " << e.text << endl;
      continue;
    }
  }
}

