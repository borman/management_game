#ifndef SESSION_H
#define SESSION_H

#include "StdLib.h"
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

    String playerName() const { return m_thisPlayer; }

    Vector<Stanza> execCommand(const Stanza &command);
  private:
    void waitForState(const String &nextState);
    void processTextMessage(const Stanza &stanza);
    void processGameData(const Stanza &stanza);

    Connection m_conn;
    GameInfo m_gameInfo;
    Queue<Stanza> m_eventQueue;
    String m_thisPlayer;
};

#endif // SESSION_H
