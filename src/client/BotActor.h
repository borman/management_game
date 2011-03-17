#ifndef BOTACTOR_H
#define BOTACTOR_H

#include "Actor.h"

class BotActor: public Actor
{
  public:
    virtual void onGameStart(Session *session);
    virtual void onTurn(Session *session);
  private:
    unsigned int roundCounter;
};

#endif // BOTACTOR_H


