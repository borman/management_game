#include <cassert>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include "Exceptions.h"
#include "Stanza.h"
#include "BotActor.h"
#include "Session.h"
#include "Term.h"

using namespace std;

void BotActor::onGameStart(Session *session)
{
  assert(session != NULL);
  
  roundCounter = 0;
}

void BotActor::onTurn(Session *session)
{
  assert(session != NULL);

  roundCounter++;
  cout << endl << Term::SetBold
       << Term::Brown("<=----------") << "      Round " 
       << setw(5) << left << roundCounter << right 
       << Term::Brown(" ---------=>") << Term::SetRegular << endl << endl;

  session->gameInfo().printTransactions(cout);
  session->gameInfo().printPlayers(cout);
  session->gameInfo().printMarket(cout);

  const MarketState &market = session->gameInfo().market();

  unsigned int rawToBuy = min(market.rawCount(), 2U); 
  unsigned int rawPrice = market.rawPrice();

  unsigned int productToSell = min(market.productCount(), 2U);
  unsigned int productPrice = market.productPrice();

  unsigned int productToProduce = 2;

  session->buy(rawToBuy, rawPrice);
  session->sell(productToSell, productPrice);
  session->produce(productToProduce);
}


