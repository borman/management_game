#include <cassert>
#include <cstdio>
#include "Exceptions.h"
#include "Stanza.h"
#include "BotActor.h"
#include "Session.h"
#include "StdLib.h"
#include "Term.h"
#include "Builtin.h"

/** Scripting API
 * [RawCount, RawPrice, ProductCount, ProductPrice] = marketState []
 * [Status, Message] = buy [Count, Price]
 * [Status, Message] = sell [Count, Price]
 * [Status, Message] = produce Count
 */

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
  m_executor.addBuiltin(&m_basis);
  m_executor.addBuiltin(this);
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

void BotActor::buy(ListedBuiltin *l_self, Context &context)
{
  execCommand(l_self, context, do_buy);
}

void BotActor::sell(ListedBuiltin *l_self, Context &context)
{
  execCommand(l_self, context, do_sell);
}

void BotActor::produce(ListedBuiltin *l_self, Context &context)
{
  execCommand(l_self, context, do_produce);
}

// ======= Helpers

void BotActor::execCommand(ListedBuiltin *l_self, Context &context, Command cmd)
{
  BotActor *self = static_cast<BotActor *>(l_self);

  bool result = true;
  Atom message("Ok", context.strings);
  try
  {
    cmd(self->m_thisSession, context);
  }
  catch (const CommandException &e)
  {
    result = false;
    message = Atom(e.text.c_str(), context.strings);
  }
  context.push(Value::TupOpen);
  context.push(result);
  context.push(message);
  context.push(Value::TupClose);
}

void BotActor::do_buy(Session *session, Context &context)
{
  context.pop(Value::TupClose);
  int price = context.pop(Value::Int).asInt();
  int count = context.pop(Value::Int).asInt();
  context.pop(Value::TupOpen);
  session->buy(count, price);
}

void BotActor::do_sell(Session *session, Context &context)
{
  context.pop(Value::TupClose);
  int price = context.pop(Value::Int).asInt();
  int count = context.pop(Value::Int).asInt();
  context.pop(Value::TupOpen);
  session->sell(count, price);
}

void BotActor::do_produce(Session *session, Context &context)
{
  int count = context.pop(Value::Int).asInt();
  session->produce(count);
}

