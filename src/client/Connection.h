#ifndef CONNECTION_H
#define CONNECTION_H

#include "TextBuffer.h"
#include "Stanza.h"
#include "Queue.h"

class Connection
{
  public:
    Connection(const char *host, short port);
    ~Connection();

    Connection &operator<<(const MakeStanza &stanza);
    Connection &operator>>(Stanza *&stanza);

  private:
    void connect(const char *host, short port);
    void readMoreData();

    int sock_fd;
    TextBuffer in_buffer;
    Queue<Stanza> in_queue;
};

#endif // CONNECTION_H
