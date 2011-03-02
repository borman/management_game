#ifndef SESSION_H
#define SESSION_H

#include "Stanza.h"
#include "StanzaQueue.h"
#include "Connection.h"
#include "GameInfo.h"

class Session
{
  public:
    Session(const char *host, short port);

    // Main loop
    void run();

    // Protocol
    void authPlayer(const char *name);
    void setReady(bool is_ready = true);
    void requestBuy(unsigned int count, unsigned int price);
    void requestSell(unsigned int count, unsigned int price);
    void requestProduce(unsigned int count);
    void requestBuild(unsigned int count);

    const GameInfo *gameInfo() const { return &game_info; }

  protected:
    virtual void onStateAuth() {}
    virtual void onStateLobby() {}
    virtual void onStateLobbyReady() {}
    virtual void onStateGame() {}
    virtual void onStateGameActive() {}

  private:
    void onStanza(const Stanza &st);
    void executeCommand(const MakeStanza &cmd);

    Connection connection;
    StanzaQueue event_queue;
    GameInfo game_info;
};

#endif // SESSION_H

