#ifndef GAMEINFO_H
#define GAMEINFO_H

class Player
{
  public:
    Player()
      : m_id(0), m_name(), m_balance(0), 
        m_raw(0), m_product(0), m_factories(0) {}

    size_t id() { return m_id; }
    std::string name() const { return m_name; }

    int balance() const { return m_balance; }
    unsigned int rawCount() const { return m_raw; }
    unsigned int productCount() const { return m_product; }
    unsigned int factoryCount() const { return m_factories; }

  private:
    size_t m_id;
    std::string m_name;
    int m_balance;
    unsigned int m_raw;
    unsigned int m_product;
    unsigned int m_factories;
};

class Transaction
{
  public:
    enum Type
    {
      Raw,
      Product
    };

    Transaction(Type t, unsigned int id, 
                unsigned int count, unsigned int price)
      : m_type(t), m_id(id), m_count(count), m_price(price) {}

    Type type() const { return m_type; }
    size_t playerId() const { return m_playerId; }
    unsigned int count() const { return m_count; }
    unsigned int price() { return m_price; }

  private:
    Type m_type;
    size_t m_playerId;
    unsigned int m_count;
    unsigned int m_price;
};

class MarketState
{
  public:
    unsigned int rawCount() const { return m_rawCount; }
    unsigned int rawPrice() const { return m_rawPrice; }
    unsigned int productCount() const { return m_productCount; }
    unsigned int productPrice() const { return m_productPrice; }
    
  private:
    unsigned int m_rawCount;
    unsigned int m_rawPrice;
    unsigned int m_productCount;
    unsigned int m_productPrice;
};

class GameInfo 
{
  public:
    size_t playerCount() const { return players.size(); }
    
    const Player &player(size_t id) const { return players[id]; }
    const Player &player(const std::string &name) const { return players[playerIdByName[name]]; }

    const vector<Transaction> &transactions() const { return lastTransactions; }
    const MarketState &market() { return marketState; }
  private:
    vector<Player> players;
    std::map<std::string, size_t> playerIdByName;

    vector<Transaction> lastTransactions;
    MarketState marketState;
};

#endif // GAMEINFO_H
