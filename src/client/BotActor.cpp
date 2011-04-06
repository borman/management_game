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
  BotActor *self = static_cast<BotActor *>(l_self);

  bool result = true;
  Atom message("Ok", context.strings);
  try
  {
    context.pop(Value::TupClose);
    int price = context.pop(Value::Int).asInt();
    int count = context.pop(Value::Int).asInt();
    context.pop(Value::TupOpen);
    self->m_thisSession->buy(count, price);
  }
  catch (const CommandException &e)
  {
    result = false;
    message = Atom(e.text.c_str(), context.strings);
  }
  context.push(Value::TupOpen);
  context.push(result);
  context.push(Value(Value::String, message.id()));
  context.push(Value::TupClose);
}

void BotActor::sell(ListedBuiltin *l_self, Context &context)
{
  BotActor *self = static_cast<BotActor *>(l_self);

  bool result = true;
  Atom message("Ok", context.strings);
  try
  {
    context.pop(Value::TupClose);
    int price = context.pop(Value::Int).asInt();
    int count = context.pop(Value::Int).asInt();
    context.pop(Value::TupOpen);
    self->m_thisSession->sell(count, price);
  }
  catch (const CommandException &e)
  {
    result = false;
    message = Atom(e.text.c_str(), context.strings);
  }
  context.push(Value::TupOpen);
  context.push(result);
  context.push(Value(Value::String, message.id()));
  context.push(Value::TupClose);
}

void BotActor::produce(ListedBuiltin *l_self, Context &context)
{
  BotActor *self = static_cast<BotActor *>(l_self);

  bool result = true;
  Atom message("Ok", context.strings);
  try
  {
    int count = context.pop(Value::Int).asInt();
    self->m_thisSession->produce(count);
  }
  catch (const CommandException &e)
  {
    result = false;
    message = Atom(e.text.c_str(), context.strings);
  }
  context.push(Value::TupOpen);
  context.push(result);
  context.push(Value(Value::String, message.id()));
  context.push(Value::TupClose);
}

