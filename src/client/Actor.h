#ifndef ACTOR_H
#define ACTOR_H

class Session;

class Actor
{
  public:
    virtual void onGameStart(Session *session) = 0;
    virtual void onTurn(Session *session) = 0;
};

#endif // ACTOR_H
