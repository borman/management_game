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
    Session(const std::string &host, unsigned short port);

    void login(NameGenerator *namegen);
    void playGame(Actor *actor);

    const GameInfo &gameInfo() const { return gameInfo; }

    // Commands
    void buy(unsigned int count, unsigned int price);
    void sell(unsigned int count, unsigned int price);
    void build(unsigned int count); 
    void signalReady(bool ready = true);

  private:
    void waitForState(const string &nextState);
    vector<Stanza> execCommand(const Stanza &command);
    void processTextMessage(const Stanza &stanza);
    void processGameData(const Stanza &stanza);

    Connection conn;
    GameInfo gameInfo;
    queue<Stanza> eventQueue;
};

#endif // SESSION_H
