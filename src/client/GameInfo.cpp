#include <cstdlib>
#include <cassert>
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
  else if (st[1] == "bankrupt")
  {
    m_players[m_playerIdByName[st[2]]].m_alive = false;
  }
}

void GameInfo::updatePlayerList(const vector<Stanza> &stanzas)
{
  if (m_players.empty())
  {
    // Fill player list
    m_players.resize(stanzas.size());
    for (size_t i=0; i<stanzas.size(); i++)
    {
      m_players[i].m_id = i;
      m_players[i].m_name = stanzas[i][1];
      m_playerIdByName[stanzas[i][1]] = i;
    }
  }

  for (size_t i=0; i<stanzas.size(); i++)
  {
    const string &playerName = stanzas[i][1];
    unsigned int balance = str2uint(stanzas[i][3]);
    unsigned int n_factories = str2uint(stanzas[i][4]);
    unsigned int n_raw = str2uint(stanzas[i][5]);
    unsigned int n_product = str2uint(stanzas[i][6]);
    size_t playerId = m_playerIdByName[playerName];
    m_players[playerId].m_balance = balance; 
    m_players[playerId].m_raw = n_raw; 
    m_players[playerId].m_product = n_product; 
    m_players[playerId].m_factories = n_factories; 
  }
}

void GameInfo::printPlayers(ostream &os) const
{
  os << "|=----------    Players       -+---------" << endl;
  os << "| Balance  Fact.  Prod.    Raw | Name" << endl;
  for (size_t i=0; i<m_players.size(); i++)
  {
    const Player &player = m_players[i];
    if (player.alive())
      os << "| " 
         << Term::SetGreen << Term::SetBold
         << setw(7) << player.balance() 
         << setw(7) << player.factoryCount() 
         << setw(7) << player.productCount()
         << setw(7) << player.rawCount()
         << Term::ResetColor << Term::SetRegular
         << " | " 
         << Term::SetGreen << Term::SetBold
         << player.name() 
         << Term::ResetColor << Term::SetRegular
         << endl;
    else
      os << "|                              | " 
         << Term::SetBlack << Term::SetBold
         << player.name() 
         << Term::ResetColor << Term::SetRegular
         << endl;
  }
  os << "|=-----------------------------+---------" << endl;
} 

void GameInfo::printTransactions(ostream &os) const
{
  os << "|=----------    Auctions      -+---------" << endl;
  os << "|    Type    Count     Price   | Player" << endl;
  for (size_t i=0; i<m_transactions.size(); i++)
  {
    const Transaction &t = m_transactions[i];
    if (t.type() == Transaction::AuctionProduct 
     || t.type() == Transaction::AuctionRaw)
      os << "| " 
         << Term::SetGreen << Term::SetBold
         << setw(7) << (t.type()==Transaction::AuctionRaw? "Raw": "Product")
         << setw(9) << t.count() 
         << setw(10) << t.price()
         << Term::ResetColor << Term::SetRegular
         << "   | " 
         << Term::SetGreen << Term::SetBold
         << player(t.playerId()).name() 
         << Term::ResetColor << Term::SetRegular
         << endl;
  }
  os << "|=-----------------------------+---------" << endl;
} 

void GameInfo::printMarket(ostream &os) const
{
  os << "|=----------   Market state   ---------=|" << endl;
  os << "|---     Raw     ---|---   Product   ---|" << endl;
  os << "|    Count    Price |    Count    Price |" << endl;
  os << "|" << Term::SetBold << Term::SetGreen
            << setw(9) << m_marketState.rawCount() 
            << setw(9) << m_marketState.rawPrice() 
            << Term::ResetColor << Term::SetRegular 
            << " |"
            << Term::SetBold << Term::SetGreen
            << setw(9) << m_marketState.productCount()
            << setw(9) << m_marketState.productPrice()
            << Term::ResetColor << Term::SetRegular 
            << " |" << endl;
  os << "|=-------------------------------------=|" << endl;
}
