#ifndef CONNECTION_H
#define CONNECTION_H

#include "SocketEventLoop.h"
#include "Command.h"

class Connection: public SocketEventLoop
{
  public:
    Connection(const char *host, short port);
    ~Connection();
  protected:
    virtual void onTextMessage(const Command &cmd);
    virtual void onStateChange(const Command &cmd);
    virtual void onGameData(const Command &cmd);

    // reimplemented from SocketEventLoop
    virtual void onLineReceived(const char *line); 
};

#endif // CONNECTION_H

