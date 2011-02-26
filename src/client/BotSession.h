#ifndef BOTSESSION_H
#define BOTSESSION_H

#include "Session.h"

class BotSession: public Session
{
  public:
    BotSession(const char *host, short port);

  protected:
    // reimplemented from Session
    virtual void onStateAuth();
    virtual void onStateLobby();
};

#endif // BOTSESSION_H
