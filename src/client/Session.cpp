#include <cstdio>

#include "Session.h"
#include "Exceptions.h"

Session::Session(const char *host, short port)
  : connection(host, port)
{
}

void Session::run()
{
  while (true)
  {
    Stanza *stanza;
    while (!event_queue.isEmpty())
    {
      event_queue >> stanza;
      onStanza(*stanza);
      delete stanza;
    }
    connection >> stanza;  
    onStanza(*stanza);
    delete stanza;
  }
}

void Session::authPlayer(const char *name)
{
  executeCommand(MakeStanza("auth", "player", name));
}

void Session::setReady(bool is_ready)
{
  executeCommand(MakeStanza(is_ready? "ready" : "notready"));
}

void Session::onStanza(const Stanza &st)
{
  switch (st.type())
  {
    case Stanza::TextMessage:
      printf("[Message %s] > %s\n", st[0], st[1]);
      break;

    case Stanza::StateChange:
      if (st.match("auth")) 
        onStateAuth();
      else if (st.match("lobby")) 
        onStateLobby();
      else if (st.match("lobby_ready")) 
        onStateLobbyReady();
      else if (st.match("game")) 
        onStateGame();
      else if (st.match("game_active")) 
        onStateGameActive();
      break;

    case Stanza::GameData:
      break;

    default:
      break;
  }
}

void Session::executeCommand(const MakeStanza &cmd)
{
  connection << cmd;
  bool processing = true;
  while (processing)
  {
    Stanza *stanza;
    connection >> stanza;
    if (stanza->type() != Stanza::Regular)
      event_queue << stanza; // Postpone
    else if (stanza->match("ack"))
    {
      processing = false;
      delete stanza;
    }
    else if (stanza->match("fail"))
    {
      delete stanza;
      throw Exception("Command fail");
    }
    else
    {
      // Command data
      delete stanza;
    }
  }
}
