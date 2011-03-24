extern "C" 
{
# include <unistd.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/select.h>
# include <sys/un.h>
# include <netinet/ip.h>
# include <arpa/inet.h>
}
#include <cerrno>
#include <cstring>

#include "Connection.h"
#include "Exceptions.h"
#include "Term.h"

// #define LOG_TRAFFIC

using namespace std;

Connection::Connection(const Address &addr)
  : sock_fd(-1)
{
  sock_fd = addr.connect();
}

Connection::~Connection()
{
  ::close(sock_fd);
}

Connection &Connection::operator<<(const Stanza &stanza)
{
  const std::string text = stanza.toString();
#ifdef LOG_TRAFFIC
  cout << Term::SetBold << Term::Green("[Socket] << ") 
       << Term::SetRegular << text << endl;
#endif

  const char *data = text.c_str();
  const size_t length = text.length();
  size_t total_sent = 0;
  while (total_sent < length)
  {
    const ssize_t nsent = ::write(sock_fd, data+total_sent, length-total_sent);
    if (nsent > 0)
      total_sent += nsent;
    else if (-1 == nsent)
      throw SocketException("write");
    else
      throw SocketException("write:eof");
  }
  return *this;
}

Connection &Connection::operator>>(Stanza &stanza)
{
  if (in_queue.empty())
    readMoreData();
  stanza = in_queue.front();
  in_queue.pop();
  return *this;
}

void Connection::readMoreData()
{
  static const size_t BUF_SIZE = 512;
  char buf[BUF_SIZE];
  while (in_queue.empty())
  {
    const ssize_t nread = ::read(sock_fd, buf, BUF_SIZE);
    if (nread>0)
    {
      for (size_t i=0; i < static_cast<size_t>(nread); i++)
        if (buf[i] == '\n')
        {
#ifdef LOG_TRAFFIC
          cout << Term::SetBold << Term::Cyan("[Socket] >> ") 
               << Term::SetRegular << in_buffer << endl;
#endif
          in_queue.push(Stanza::parse(in_buffer));
          in_buffer.clear();
        }
        else
          in_buffer += buf[i];
    }
    else if (nread<0)
      throw SocketException("read");
    else
      throw SocketException("read:eof");
  }
}

int InetAddress::connect() const
{
  int sock_fd = ::socket(AF_INET, SOCK_STREAM, 0);
  if (-1 == sock_fd)
    throw SocketException("socket");

  struct sockaddr_in sa;
  sa.sin_family = AF_INET;
  sa.sin_port = htons(m_port);
  if (0 == ::inet_aton(m_host.c_str(), &sa.sin_addr))
  {
    close(sock_fd);
    throw SocketException("inet_aton");
  }

  if (-1 == ::connect(sock_fd, (struct sockaddr *)&sa, sizeof(sa)))
  {
    close(sock_fd);
    throw SocketException("connect");
  }

  return sock_fd;
}

int UnixAddress::connect() const
{
  int sock_fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
  if (-1 == sock_fd)
    throw SocketException("socket");

  struct sockaddr_un sa;
  memset(&sa, 0, sizeof(sa));
  sa.sun_family = AF_UNIX;
  strcpy(sa.sun_path, m_path.c_str());

  if (-1 == ::connect(sock_fd, (struct sockaddr *)&sa, sizeof(sa)))
  {
    close(sock_fd);
    throw SocketException("connect");
  }

  return sock_fd;
}
