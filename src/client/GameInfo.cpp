#include <cstdlib>
#include <cassert>
#include <iostream>
#include <iomanip>
#include "GameInfo.h"
#include "Term.h"

using namespace std;

static unsigned int str2uint(const string &str)
{
  return strtoul(str.c_str(), NULL, 10);
}

static Transaction::Type parseTransactionType(const string &type, 
                                             const string &subtype)
{
  Transaction::Type t = Transaction::Invalid;
  if (type == "auction")
  {
    if (subtype == "raw")
      t = Transaction::AuctionRaw;
    else if (subtype == "product")
      t = Transaction::AuctionProduct;
  }
  else if (type == "expense")
  {
    if (subtype == "factory")
      t = Transaction::ExpenseFactory;
    else if (subtype == "product")
      t = Transaction::ExpenseProduct;
    else if (subtype == "raw")
      t = Transaction::ExpenseRaw;
    else if (subtype == "production")
      t = Transaction::ExpenseProduction;
    else if (subtype == "construction_begin")
      t = Transaction::ExpenseConstructionBegin;
    else if (subtype == "construction_end")
      t = Transaction::ExpenseConstructionEnd;
  }

  assert(t != Transaction::Invalid);
  return t;
}

void GameInfo::consume(const Stanza &st)
{
  if (st[1] == "market")
  {
    m_marketState.m_rawCount = str2uint(st[2]);
    m_marketState.m_rawPrice = str2uint(st[3]);
    m_marketState.m_productCount = str2uint(st[4]);
    m_marketState.m_productPrice = str2uint(st[5]);
  } 
  else if (st[1] == "finance")
  {
    m_transactions.push_back(Transaction(
        parseTransactionType(st[3], st[4]),
        m_playerIdByName[st[2]],
        str2uint(st[5]),
        str2uint(st[6])));
  }
}

void GameInfo::updatePlayerList(const vector<Stanza> &stanzas)
{

}

ostream &operator<<(ostream &os, const MarketState &market)
{
  os << "|=----------   Market state   ---------=|" << endl;
  os << "|---     Raw     ---|---   Product   ---|" << endl;
  os << "|    Count    Price |    Count    Price |" << endl;
  os << "|" << Term::SetBold << Term::SetGreen
            << setw(9) << market.rawCount() 
            << setw(9) << market.rawPrice() 
            << Term::ResetColor << Term::SetRegular 
            << " |"
            << Term::SetBold << Term::SetGreen
            << setw(9) << market.productCount()
            << setw(9) << market.productPrice()
            << Term::ResetColor << Term::SetRegular 
            << " |" << endl;
  os << "|=-------------------------------------=|" << endl;
  return os;
}
