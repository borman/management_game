#include <cstdio>
#include <cassert>
#include <cstring>

#include "Command.h"
#include "Connection.h"

Connection::Connection(const char *host, short port)
  : SocketEventLoop(host, port)
{
}

Connection::~Connection()
{
}

void Connection::onLineReceived(const char *line)
{
  Command cmd(line);
  switch (cmd.type())
  {
    case Command::Regular:
      break;
    case Command::StateChange:
      onStateChange(cmd);
      break;
    case Command::GameData:
      onGameData(cmd);
      break;
    case Command::TextMessage:
      onTextMessage(cmd);
      break;
  }
}

void Connection::onTextMessage(const Command &cmd)
{
  assert(cmd.size() == 2);
  printf("Message[%s] >> %s\n", cmd[0], cmd[1]);
}

void Connection::onStateChange(const Command &cmd)
{
  assert(cmd.size() == 1);
  printf("State >> %s\n", cmd[0]);

  if (0 == strcmp(cmd[0], "auth"))
    send("auth player Bot\n");
}

void Connection::onGameData(const Command &cmd)
{
  printf("GameData >> ");
  for (size_t i=0; i<cmd.size(); i++)
    printf("[%s] ", cmd[i]);
  printf("\n");
}
