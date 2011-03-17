#ifndef SESSION_H
#define SESSION_H

#include <string>
#include <vector>
#include <queue>
#include "Actor.h"
#include "Connection.h"
#include "GameInfo.h"
#include "NameGenerator.h"
#include "Stanza.h"

class Session 
{
  public:
    Session(const Address &addr);

    void login(NameGenerator *namegen);
    void playGame(Actor *actor);

    const GameInfo &gameInfo() const { return m_gameInfo; }

    // Commands
    void buy(unsigned int count, unsigned int price);
    void sell(unsigned int count, unsigned int price);
    void produce(unsigned int count);
    void build(unsigned int count); 
    void signalReady(bool ready = true);

    std::vector<Stanza> execCommand(const Stanza &command);
  private:
    void waitForState(const std::string &nextState);
    void processTextMessage(const Stanza &stanza);
    void processGameData(const Stanza &stanza);

    Connection m_conn;
    GameInfo m_gameInfo;
    std::queue<Stanza> m_eventQueue;
};

#endif // SESSION_H
