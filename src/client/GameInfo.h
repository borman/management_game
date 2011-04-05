#ifndef GAMEINFO_H
#define GAMEINFO_H

#include <cstdio>
#include "Stanza.h"
#include "StdLib.h"

class Player
{
  public:
    Player()
      : m_id(0), m_name(), m_balance(0), 
        m_raw(0), m_product(0), m_factories(0), m_alive(true) {}

    size_t id() { return m_id; }
    String name() const { return m_name; }

    bool alive() const { return m_alive; }

    int balance() const { return m_balance; }
    unsigned int rawCount() const { return m_raw; }
    unsigned int productCount() const { return m_product; }
    unsigned int factoryCount() const { return m_factories; }

  private:
    size_t m_id;
    String m_name;
    int m_balance;
    unsigned int m_raw;
    unsigned int m_product;
    unsigned int m_factories;
    bool m_alive;

    friend class GameInfo;
};

class Transaction
{
  public:
    enum Type
    {
      Invalid,
      AuctionRaw,
      AuctionProduct,
      ExpenseFactory,
      ExpenseProduct,
      ExpenseRaw,
      ExpenseProduction,
      ExpenseConstructionBegin,
      ExpenseConstructionEnd
    };

    Transaction(Type t=Invalid, unsigned int player_id=0, 
                unsigned int count=0, unsigned int price=0)
      : m_type(t), m_playerId(player_id), m_count(count), m_price(price) {}

    Type type() const { return m_type; }
    size_t playerId() const { return m_playerId; }
    unsigned int count() const { return m_count; }
    unsigned int price() const { return m_price; }

  private:
    Type m_type;
    size_t m_playerId;
    unsigned int m_count;
    unsigned int m_price;
};

class MarketState
{
  public:
    MarketState()
      : m_rawCount(0), m_rawPrice(0), 
        m_productCount(0), m_productPrice(0) {}

    unsigned int rawCount() const { return m_rawCount; }
    unsigned int rawPrice() const { return m_rawPrice; }
    unsigned int productCount() const { return m_productCount; }
    unsigned int productPrice() const { return m_productPrice; }
    
    void prettyPrint() const;
  private:
    unsigned int m_rawCount;
    unsigned int m_rawPrice;
    unsigned int m_productCount;
    unsigned int m_productPrice;

    friend class GameInfo;
};

class GameInfo 
{
  public:
    size_t playerCount() const { return m_players.size(); }
    
    const Player &player(size_t id) const 
      { return m_players[id]; }
    const Player &player(const String &name) const 
      { return m_players[m_playerIdByName.at(name)]; }

    const Vector<Transaction> &transactions() const
      { return m_transactions; }
    const MarketState &market() const 
      { return m_marketState; }

    void printPlayers(FILE *out) const;
    void printMarket(FILE *out) const;
    void printTransactions(FILE *out) const;

    void consume(const Stanza &st);
    void clearTransactions() { m_transactions.clear(); }
    void updatePlayerList(const Vector<Stanza> &stanzas);
  private:
    Vector<Player> m_players;
    Map<String, size_t> m_playerIdByName;

    Vector<Transaction> m_transactions;
    MarketState m_marketState;
};

#endif // GAMEINFO_H
