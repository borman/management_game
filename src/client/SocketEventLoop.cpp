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

#include "SocketEventLoop.h"
#include "Exceptions.h"


SocketEventLoop::SocketEventLoop(const char *host, short port)
  : sock_fd(-1)
{
  connect(host, port);
}

SocketEventLoop::~SocketEventLoop()
{
}

void SocketEventLoop::run()
{
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(sock_fd, &fds);

  do 
  {
    int retval = ::select(sock_fd+1, &fds, NULL, NULL, NULL);
    if (-1 == retval)
    {
      if (errno != EINTR)
        throw SocketException("select");
      else
        continue;
    }
    readNewData();
  } while (true);
}

void SocketEventLoop::send(const char *text)
{
  (void)text;
}

void SocketEventLoop::readNewData()
{
  static const size_t BUF_SIZE = 512;
  char buf[BUF_SIZE];
  ssize_t nread = ::read(sock_fd, buf, BUF_SIZE);
  if (nread>0)
  {
    for (size_t i=0; i<nread; i++)
      if (buf[i] == '\n')
      {
        onLineReceived(in_buffer.c_str());
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

void SocketEventLoop::onLineReceived(const char *line)
{
  ::printf("SocketEventLoop >> %s\n", line);
}

void SocketEventLoop::connect(const char *host, short port)
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
