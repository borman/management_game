#include <cstdio>
#include <cassert>
#include "Session.h"
#include "Exceptions.h"
#include "Term.h"



static String uint2str(unsigned int u)
{
  char str[16];
  sprintf(str, "%u", u);
  return str;
}

Session::Session(const Address &addr)
  : m_conn(addr)
{
}

void Session::login(NameGenerator *namegen)
{
  waitForState("auth");
  bool success = false;
  while (!success)
  {
    try
    {
      execCommand(Stanza("auth", "player", namegen->nextName()));
      success = true;
    }
    catch (const CommandException &e)
    {
      continue;
    }
  }
  waitForState("lobby");
}

void Session::playGame(Actor *actor)
{
  signalReady();
  waitForState("lobby_ready");
  waitForState("game");
  actor->onGameStart(this);
  bool in_game = true;
  m_gameInfo = GameInfo(); // Reset game info
  while (in_game)
  {
    try
    {
      waitForState("game_active");
      m_gameInfo.updatePlayerList(execCommand(Stanza("lsgame")));
      actor->onTurn(this);
      signalReady();
      m_gameInfo.clearTransactions();
      waitForState("game");
    }
    catch (const UnexpectedStateException &e)
    {
      assert(e.text == "lobby");
      printf("The game is over.\n");
      in_game = false;
    }
  }
}


void Session::buy(unsigned int count, unsigned int price)
{
  execCommand(Stanza("buy", uint2str(count), uint2str(price)));
}

void Session::sell(unsigned int count, unsigned int price)
{
  execCommand(Stanza("sell", uint2str(count), uint2str(price)));
}

void Session::produce(unsigned int count)
{
  execCommand(Stanza("produce", uint2str(count)));
}

void Session::build(unsigned int count)
{
  execCommand(Stanza("build", uint2str(count)));
}
 
void Session::signalReady(bool ready)
{
  execCommand(Stanza(ready? "ready" : "notready"));
}


void Session::waitForState(const String &nextState)
{
  while (true) 
  {
    Stanza st;
    if (!m_eventQueue.empty())
    {
      st = m_eventQueue.front();
      m_eventQueue.pop();
    }
    else
      m_conn >> st;
    
    if (st[0] == ">")
      processTextMessage(st);
    else if (st[0] == "+")
      processGameData(st);
    else
    { 
      // Only a state change stanza will get here
      assert(st[0] == "$");
      if (st[1] != nextState)
        throw UnexpectedStateException(st[1]);
      break;
    }
  }
}

void Session::processTextMessage(const Stanza &stanza)
{
#if 0
  const String &nick = stanza[1];
  const String &text = stanza[2];

  printf("%sMessage [%s] -> %s%s\n",
      Term::SetBold, 
      nick.c_str(), text.c_str(),
      Term::SetRegular);
#endif
}

void Session::processGameData(const Stanza &stanza)
{
  // printf("GameData [%s]\n", stanza[1].c_str()); 
  m_gameInfo.consume(stanza);
}

Vector<Stanza> Session::execCommand(const Stanza &command)
{
  m_conn << command;
  Vector<Stanza> reply;
  while (true)
  {
    Stanza st;
    m_conn >> st;
    if (st[0] == "ack")
      break;
    else if (st[0] == "fail")
      throw CommandException(st[1]);
    else if (st[0] == ">")
      processTextMessage(st);
    else if (st[0] == "+" || st[0] == "$")
      m_eventQueue.push(st);
    else
      reply.push_back(st);
  }
  return reply;
}

