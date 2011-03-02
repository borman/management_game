extern "C" 
{
# include <unistd.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/select.h>
# include <netinet/ip.h>
# include <arpa/inet.h>
}
#include <cerrno>
#include <cstdio>
#include <cstring>

#include "Connection.h"
#include "Exceptions.h"


Connection::Connection(const char *host, short port)
  : sock_fd(-1)
{
  connect(host, port);
}

Connection::~Connection()
{
  ::close(sock_fd);
}

Connection &Connection::operator<<(const MakeStanza &stanza)
{
  const char *text = stanza.c_str();
  printf("[Socket] << %s", text);

  size_t length = strlen(text);
  size_t total_sent = 0;
  while (total_sent < length)
  {
    ssize_t nsent = ::write(sock_fd, text+total_sent, length-total_sent);
    if (nsent > 0)
      total_sent += nsent;
    else if (-1 == nsent)
      throw SocketException("write");
    else
      throw SocketException("write:eof");
  }
  return *this;
}

Connection &Connection::operator>>(Stanza *&stanza)
{
  if (in_queue.isEmpty())
    readMoreData();
  in_queue >> stanza;
  return *this;
}

void Connection::readMoreData()
{
  static const size_t BUF_SIZE = 512;
  char buf[BUF_SIZE];
  while (in_queue.isEmpty())
  {
    ssize_t nread = ::read(sock_fd, buf, BUF_SIZE);
    if (nread>0)
    {
      for (size_t i=0; i<static_cast<size_t>(nread); i++)
        if (buf[i] == '\n')
        {
          printf("[Socket] >> %s\n", in_buffer.c_str());
          in_queue << new Stanza(in_buffer.c_str());
          in_buffer.clear();
        }
        else
          in_buffer << buf[i];
    }
    else if (nread<0)
      throw SocketException("read");
    else
      throw SocketException("read:eof");
  }
}

void Connection::connect(const char *host, short port)
{
  sock_fd = ::socket(AF_INET, SOCK_STREAM, 0);
  if (-1 == sock_fd)
    throw SocketException("socket");

  struct sockaddr_in sa;
  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  if (0 == ::inet_aton(host, &sa.sin_addr))
    throw SocketException("inet_aton");

  if (-1 == ::connect(sock_fd, (struct sockaddr *)&sa, sizeof(sa)))
    throw SocketException("connect");
}
