#include <cassert>
#include "Exceptions.h"
#include "Stanza.h"
#include "HumanActor.h"
#include "Session.h"
#include "Term.h"



void HumanActor::onGameStart(Session *session)
{
  assert(session != NULL);
}

void HumanActor::onTurn(Session *session)
{
  assert(session != NULL);

  session->gameInfo().printPlayers(stdout);
  session->gameInfo().printMarket(stdout);

  bool turn_must_go_on = true;
  while (turn_must_go_on)
  {
    printf("%sYour turn>%s ", Term::SetBold, Term::SetRegular);
    String s;
    //getline(stdin, s); // ACHTUNG!
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
      printf("%sCommand syntax error%s\n", Term::SetRed, Term::ResetColor);
      continue;
    }
    catch (const CommandException &e)
    {
      printf("%sCommand failed: %s%s\n", Term::SetRed, e.text.c_str(), Term::ResetColor);
      continue;
    }
  }
}

