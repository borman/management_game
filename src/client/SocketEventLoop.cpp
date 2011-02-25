extern "C" 
{
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/select.h>
# include <netinet/ip.h>
# include <arpa/inet.h>
}

#include "SocketEventLoop.h"


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
  }
}

void SocketEventLoop::send(const char *text)
{

}

void SocketEventLoop::onLineReceived(const char *line)
{
  ::printf("SocketEventLoop: %s\n", line);
}

void SocketEventLoop::connect(const char *host, short port)
{
  sock_fd = ::socket(AF_INET, SOCK_STREAM, 0);
  if (-1 == sock_fd)
    throw SocketException("socket");

  struct sockaddr_in sa;
  sa.sin_family = AF_INET;
  sa.sin_port = port;
  ::inet_aton(host, &sa);

  if (-1 == ::connect(sock_fd, &sa, sizeof(sa)))
    throw SocketException("connect");
}
