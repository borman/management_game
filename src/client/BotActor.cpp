#include <cassert>
#include <cstdio>
#include "Exceptions.h"
#include "Stanza.h"
#include "BotActor.h"
#include "Session.h"
#include "StdLib.h"
#include "Term.h"
#include "Builtin.h"

const ListedBuiltin::Definition BotActor::defs[] =
{
  {"marketState", BotActor::marketState},
  {"buy", BotActor::buy},
  {"sell", BotActor::sell},
  {"produce", BotActor::produce}
};

BotActor::BotActor(LoadedProgram &program)
  : ListedBuiltin(program.strings(), defs, sizeof(defs)/sizeof(ListedBuiltin::Definition)), 
  m_executor(program, program.strings()), m_basis(program.strings()), m_thisSession(NULL)
{
}

void BotActor::onGameStart(Session *session)
{
  assert(session != NULL);
  
  m_thisSession = session;
  m_executor.run("onStart");
  m_thisSession = NULL;
}

void BotActor::onTurn(Session *session)
{
  assert(session != NULL);

  m_thisSession = session;
  m_executor.run("onTurn");
  m_thisSession = NULL;
}

// ======= Scripting accessors

void BotActor::marketState(ListedBuiltin *l_self, Context &context)
{
  BotActor *self = static_cast<BotActor *>(l_self);

  // Empty argument
  context.popdelete();

  const MarketState &ms = self->m_thisSession->gameInfo().market();

  // rawCount rawPrice productCount productPrice
  context.push(Value::TupOpen);
  context.push(int(ms.rawCount()));
  context.push(int(ms.rawPrice()));
  context.push(int(ms.productCount()));
  context.push(int(ms.productPrice()));
  context.push(Value::TupClose);
}

void BotActor::buy(ListedBuiltin *self, Context &context)
{
}

void BotActor::sell(ListedBuiltin *self, Context &context)
{
}

void BotActor::produce(ListedBuiltin *self, Context &context)
{
}

