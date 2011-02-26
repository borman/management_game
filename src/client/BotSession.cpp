#include "BotSession.h"

BotSession::BotSession(const char *host, short port)
  : Session(host, port)
{
}

void BotSession::onStateAuth()
{
  authPlayer("Bot");
}

void BotSession::onStateLobby()
{
  setReady();
}

