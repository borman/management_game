#ifndef HUMANACTOR_H
#define HUMANACTOR_H

#include "Actor.h"

class HumanActor: public Actor
{
  public:
    virtual void onGameStart(Session *session);
    virtual void onTurn(Session *session);
};

#endif // HUMANACTOR_H

