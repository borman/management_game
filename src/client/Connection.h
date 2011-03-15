#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <queue>
#include "Stanza.h"

class Address
{
  public:
    virtual ~Address() {}
    virtual int connect() const = 0;
};

class InetAddress: public Address
{
  public:
    InetAddress(const std::string &host, unsigned short port)
      : m_host(host), m_port(port) {}
    virtual int connect() const;

  private:
    std::string m_host;
    unsigned short m_port;
};

class UnixAddress: public Address
{
  public:
    UnixAddress(const std::string &path)
      : m_path(path) {}
    virtual int connect() const;

  private:
    std::string m_path;
};

class Connection
{
  public:
    Connection(const Address &addr);
    ~Connection();

    Connection &operator<<(const Stanza &stanza);
    Connection &operator>>(Stanza &stanza);

  private:
    void readMoreData();

    int sock_fd;
    std::string in_buffer;
    std::queue<Stanza> in_queue;
};

#endif // CONNECTION_H
