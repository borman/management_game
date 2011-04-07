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
 * Count = nPlayers []
 * [Value1, Value2, ...] = player [Index, [Prop1, Prop2, ...]]
 * [Type, Player, Count, Price] = transaction N
 */

const ListedBuiltin::Definition BotActor::defs[] =
{
#define BUILTIN(_name) {#_name, BotActor::_name}
  BUILTIN(buy),
  BUILTIN(sell),
  BUILTIN(produce),
  BUILTIN(build),
  BUILTIN(marketState),
  BUILTIN(nPlayers),
  BUILTIN(player),
  BUILTIN(thisPlayer),
  BUILTIN(nTransactions),
  BUILTIN(transaction)
#undef BUILTIN
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
  callScripted(session, "onStart");
}

void BotActor::onTurn(Session *session)
{
  callScripted(session, "onTurn");
}

void BotActor::callScripted(Session *session, const char *fun)
{
  assert(session != NULL);

  m_thisSession = session;

  try
  {
    m_executor.run(fun);
  }
  catch (const Executor::Undefined &e)
  {
    cout.printf("Undefined %s %s in executor at %04zu\n", 
        e.type()==Executor::Undefined::Function? "function" : "variable",
        e.name().c_str(), e.addr());
    throw;
  }
  catch (const Executor::Exception &e)
  {
    cout.printf("%s in executor ar %04zu\n", e.text(), e.addr());
    throw;
  }

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

void BotActor::build(ListedBuiltin *l_self, Context &context)
{
  execCommand(l_self, context, do_build);
}

void BotActor::nPlayers(ListedBuiltin *l_self, Context &context)
{
  BotActor *self = static_cast<BotActor *>(l_self);

  // Empty argument
  context.popdelete();

  context.push(int(self->m_thisSession->gameInfo().playerCount()));
}

void BotActor::player(ListedBuiltin *l_self, Context &context)
{
  BotActor *self = static_cast<BotActor *>(l_self);

  Stack<PlayerProp> props;

  context.pop(Value::TupClose);
  context.pop(Value::TupClose);
  // Read props
  while (context.stack.top().type() != Value::TupOpen)
  {
    Atom prop(context.pop(Value::String).asString(), context.strings);
    if (prop == "Name")
      props.push(Name);
    else if (prop == "Alive")
      props.push(Alive);
    else if (prop == "Balance")
      props.push(Balance);
    else if (prop == "Raw")
      props.push(Raw);
    else if (prop == "Product")
      props.push(Product);
    else if (prop == "Factories")
      props.push(Factories);
  }
  context.pop(Value::TupOpen);
  int playerId = context.pop(Value::Int).asInt() - 1;
  context.pop(Value::TupOpen);

  const Player &player = self->m_thisSession->gameInfo().player(playerId);
  context.push(Value::TupOpen);
  while (!props.empty())
  {
    switch (props.top())
    {
      case Name:
        context.push(Atom(player.name().c_str(), context.strings));
        break;
      case Alive:
        context.push(player.alive());
        break;
      case Balance:
        context.push(player.balance());
        break;
      case Raw:
        context.push(int(player.rawCount()));
        break;
      case Product:
        context.push(int(player.productCount()));
        break;
      case Factories:
        context.push(int(player.factoryCount()));
        break;
      default:
        break;
    }
    props.pop();
  }
  context.push(Value::TupClose);
}

void BotActor::thisPlayer(ListedBuiltin *l_self, Context &context)
{
  BotActor *self = static_cast<BotActor *>(l_self);

  // Empty argument
  context.popdelete();
  
  Session *session = self->m_thisSession;
  context.push(int(session->gameInfo().player(session->playerName()).id() + 1));
}

void BotActor::nTransactions(ListedBuiltin *l_self, Context &context)
{
  BotActor *self = static_cast<BotActor *>(l_self);

  // Empty argument
  context.popdelete();

  context.push(int(self->m_thisSession->gameInfo().transactions().size()));
}

void BotActor::transaction(ListedBuiltin *l_self, Context &context)
{
  BotActor *self = static_cast<BotActor *>(l_self);

  // Empty argument
  int n = context.pop(Value::Int).asInt() - 1;

  const Transaction &tr = self->m_thisSession->gameInfo().transactions()[n];
  const char *s_type;
  switch (tr.type())
  {
#define TR_TYPE(_T) case Transaction::_T: s_type = #_T; break
      TR_TYPE(AuctionRaw);
      TR_TYPE(AuctionProduct);
      TR_TYPE(ExpenseFactory);
      TR_TYPE(ExpenseProduct);
      TR_TYPE(ExpenseRaw);
      TR_TYPE(ExpenseProduction);
      TR_TYPE(ExpenseConstructionBegin);
      TR_TYPE(ExpenseConstructionEnd); 
#undef TR_TYPE
    default: s_type = "Invalid";
  }

  context.push(Value::TupOpen);
  context.push(Atom(s_type, context.strings)); // Type
  context.push(int(tr.playerId() + 1)); // Player
  context.push(int(tr.count())); // Count
  context.push(int(tr.price())); // Price
  context.push(Value::TupClose);
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

void BotActor::do_build(Session *session, Context &context)
{
  int count = context.pop(Value::Int).asInt();
  session->build(count);
}


