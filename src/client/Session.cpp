#include "Session.h"

static std::string uint2str(unsigned int u)
{
  char str[16];
  sprintf(str, "%u", u);
  return str;
}

Session::Session(const std::string &host, unsigned short port)
{
}

void Session::login(NameGenerator *namegen)
{
}

void Session::playGame(Actor *actor)
{
}


void Session::buy(unsigned int count, unsigned int price)
{
  execCommand(Stanza("buy", uint2str(count), uint2str(price)));
}

void Session::sell(unsigned int count, unsigned int price)
{
  execCommand(Stanza("sell", uint2str(count), uint2str(price)));
}

void Session::build(unsigned int count)
{
  execCommand(Stanza("build", uint2str(count)));
}
 
void Session::signalReady(bool ready = true)
{
  execCommand(Stanza(ready? "ready" : "notready"));
}


void Session::waitForState(const string &nextState)
{
  while (true) 
  {
    Stanza st;
    if (!eventQueue.empty())
    {
      st = eventQueue.front();
      eventQueue.pop();
    }
    else
      conn >> st;

    if (st[0] == ">")
      processTextMessage(st);
    else if (st[0] == "+")
      processGameData(st);
    else
    { 
      // Only a state change stanza will get here
      assert(st[0] == "$");
      assert(st[1] == nextState);
      break;
    }
  }
}

void processTextMessage(const Stanza &stanza)
{
  std::string nick = stanza[1];
  std::string text = stanza[2];

  printf("Message [%s] -> %s\n", nick.c_str(), text.c_str());
}

void processGameData(const Stanza &stanza)
{
  printf("GameData [%s]\n", stanza[1]); 
}

vector<Stanza> Session::execCommand(const Stanza &command)
{
  conn << command;
  while (true)
  {
    Stanza st;
    conn >> st;
    if (st[0] == ">" || st[0] == "+" || st[0] == "$")
      eventQueue.push(st);
    
  }
}

