#ifndef DUMBACTOR_H
#define DUMBACTOR_H

#include "Actor.h"

class DumbActor: public Actor
{
  public:
    virtual void onGameStart(Session *session);
    virtual void onTurn(Session *session);
  private:
    unsigned int roundCounter;
};

#endif // DUMBACTOR_H


