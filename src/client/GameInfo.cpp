#include <cstdlib>
#include <cassert>
#include "GameInfo.h"
#include "Term.h"

static unsigned int str2uint(const String &str)
{
  return strtoul(str.c_str(), NULL, 10);
}

static Transaction::Type parseTransactionType(const String &type, 
                                             const String &subtype)
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

void GameInfo::updatePlayerList(const Vector<Stanza> &stanzas)
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
    const String &playerName = stanzas[i][1];
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

void GameInfo::printPlayers(FILE *out) const
{
  fprintf(out, "|=----------    Players       -+---------\n");
  fprintf(out, "| Balance  Fact.  Prod.    Raw | Name\n");
  for (size_t i=0; i<m_players.size(); i++)
  {
    const Player &player = m_players[i];
    if (player.alive())
      fprintf(out, "| %s%s%7u%7u%7u%7u%s%s | %s%s%s%s%s\n",
          Term::SetGreen, Term::SetBold,
          player.balance(), player.factoryCount(), player.productCount(), player.rawCount(),
          Term::ResetColor, Term::SetRegular,
          Term::SetGreen, Term::SetBold,
          player.name().c_str(),
          Term::ResetColor, Term::SetRegular);
    else
      fprintf(out, "|                              | %s%s%s%s%s\n", 
          Term::SetBlack, Term::SetBold,
          player.name().c_str(),
          Term::ResetColor, Term::SetRegular);
  }
  fprintf(out, "|=-----------------------------+---------\n");
} 

void GameInfo::printTransactions(FILE *out) const
{
  fprintf(out, "|=----------    Auctions      -+---------\n");
  fprintf(out, "|    Type    Count     Price   | Player\n");
  for (size_t i=0; i<m_transactions.size(); i++)
  {
    const Transaction &t = m_transactions[i];
    if (t.type() == Transaction::AuctionProduct 
        || t.type() == Transaction::AuctionRaw)
      fprintf(out, "| %s%s%7s%9u%10u%s%s   | %s%s%s%s%s\n",
          Term::SetGreen, Term::SetBold,
          (t.type()==Transaction::AuctionRaw? "Raw": "Product"),
          t.count(), t.price(),
          Term::ResetColor, Term::SetRegular,
          Term::SetGreen, Term::SetBold,
          player(t.playerId()).name().c_str(),
          Term::ResetColor, Term::SetRegular);
  }
  fprintf(out, "|=-----------------------------+---------\n");
} 

void GameInfo::printMarket(FILE *out) const
{
  fprintf(out, "|=----------   Market state   ---------=|\n");
  fprintf(out, "|---     Raw     ---|---   Product   ---|\n");
  fprintf(out, "|    Count    Price |    Count    Price |\n");
  fprintf(out, "|%s%s%9u%9u%s%s |%s%s%9u%9u%s%s |\n", 
            Term::SetBold, Term::SetGreen,
            m_marketState.rawCount(), m_marketState.rawPrice(),
            Term::ResetColor, Term::SetRegular,
            Term::SetBold, Term::SetGreen,
            m_marketState.productCount(), m_marketState.productPrice(),
            Term::ResetColor, Term::SetRegular);
  fprintf(out, "|=-------------------------------------=|\n");
}
