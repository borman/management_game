#ifndef CONNECTION_H
#define CONNECTION_H

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
    InetAddress(const String &host, unsigned short port)
      : m_host(host), m_port(port) {}
    virtual int connect() const;

  private:
    String m_host;
    unsigned short m_port;
};

class UnixAddress: public Address
{
  public:
    UnixAddress(const String &path)
      : m_path(path) {}
    virtual int connect() const;

  private:
    String m_path;
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
    String in_buffer;
    Queue<Stanza> in_queue;
};

#endif // CONNECTION_H
