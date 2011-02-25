#ifndef SOCKETEVENTLOOP_H
#define SOCKETEVENTLOOP_H

#include "TextBuffer.h"

class SocketEventLoop
{
  public:
    SocketEventLoop(const char *host, short port);
    ~SocketEventLoop();

    void run();

    void send(const char *text);

  protected:
    virtual void onLineReceived(const char *line);

  private:
    void connect(const char *host, short port);
    void readNewData();

    int sock_fd;
    TextBuffer in_buffer;
};

#endif // ESOCKETEVENTLOOP_H
