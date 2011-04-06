#ifndef BOTACTOR_H
#define BOTACTOR_H

#include "Actor.h"
#include "LoadedProgram.h"
#include "Executor.h"
#include "Builtin.h"
#include "BasicBuiltin.h"
#include "Context.h"

class BotActor: public Actor, public ListedBuiltin
{
  public:
    BotActor(LoadedProgram &program);

    virtual void onGameStart(Session *session);
    virtual void onTurn(Session *session);
  private:
    typedef void (*Command)(Session *session, Context &context);

    // [rawCount, rawPrice, productCount, productPrice]
    static void marketState(ListedBuiltin *self, Context &context);
    
    static void execCommand(ListedBuiltin *self, Context &context, Command cmd);
    static void buy(ListedBuiltin *self, Context &context);
    static void sell(ListedBuiltin *self, Context &context);
    static void produce(ListedBuiltin *self, Context &context);

    static void do_buy(Session *session, Context &context);
    static void do_sell(Session *session, Context &context);
    static void do_produce(Session *session, Context &context);

    static const ListedBuiltin::Definition defs[];
    Executor m_executor;
    BasicBuiltin m_basis;
    Session *m_thisSession;
};

#endif // BOTACTOR_H


