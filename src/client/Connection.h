#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <queue>
#include "Stanza.h"

class Connection
{
  public:
    Connection(const std::string &host, unsigned short port);
    ~Connection();

    Connection &operator<<(const Stanza &stanza);
    Connection &operator>>(Stanza &stanza);

  private:
    void connect(const std::string &host, unsigned short port);
    void readMoreData();

    int sock_fd;
    std::string in_buffer;
    std::queue<Stanza> in_queue;
};

#endif // CONNECTION_H
