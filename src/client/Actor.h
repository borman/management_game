#ifndef ACTOR_H
#define ACTOR_H

#include "Session.h"

class Actor
{
  public:
    virtual void onGameStart(Session &session) = 0;
    virtual void onTurn(Session &session) = 0;
};

#endif // ACTOR_H
