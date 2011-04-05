#include <cassert>
#include <cstdio>
#include "Exceptions.h"
#include "Stanza.h"
#include "BotActor.h"
#include "Session.h"
#include "StdLib.h"
#include "Term.h"



void BotActor::onGameStart(Session *session)
{
  assert(session != NULL);
  
  roundCounter = 0;
}

void BotActor::onTurn(Session *session)
{
  assert(session != NULL);

  roundCounter++;
  printf("%s%s<=----------%s      Round %-5u %s---------=>%s%s\n",
      Term::SetBold, Term::SetBrown, Term::ResetColor,
      roundCounter,
      Term::SetBrown, Term::ResetColor, Term::SetRegular);

  session->gameInfo().printTransactions(stdout);
  session->gameInfo().printPlayers(stdout);
  session->gameInfo().printMarket(stdout);

  const MarketState &market = session->gameInfo().market();

  unsigned int rawToBuy = min(market.rawCount(), 2U); 
  unsigned int rawPrice = market.rawPrice();

  unsigned int haveProduct = session->gameInfo()
    .player(session->playerName()).productCount();
  unsigned int productToSell = min(market.productCount(), haveProduct);
  unsigned int productPrice = market.productPrice();

  unsigned int productToProduce = 2;

  session->buy(rawToBuy, rawPrice);
  session->sell(productToSell, productPrice);
  session->produce(productToProduce);
}


