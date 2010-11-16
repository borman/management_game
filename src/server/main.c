/*
 * Copyright 2010, Mikhail "borman" Borisov <borisov.mikhail@gmail.com>
 *
 * This file is part of borman's management game server.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "core/socket_loop.h"
#include "core/lexer.h"
#include "core/log.h"
#include "server/server_fsm.h"


#define TCP_IN_PORT 8982


static int listen_tcp(SocketLoop *loop);
static int listen_unix(SocketLoop *loop);
static void terminate_handler(int);

static void on_client_connect(SocketLoop *loop, int fd);
static void on_incoming_message(SocketLoop *loop, int fd, const char *msg);
static void on_client_disconnect(SocketLoop *loop, int fd);


SocketLoop *main_loop;
const SocketLoopEventHandler event_handler = 
{
  on_client_connect,
  on_incoming_message,
  on_client_disconnect
};


int main()
{
  FSM *server_fsm;
  int tcp_ok;
  int unix_ok;

  message("Server started");
  main_loop = socketloop_new(&event_handler);

  signal(SIGINT, terminate_handler);
  signal(SIGHUP, terminate_handler);
  signal(SIGTERM, terminate_handler);
  
  server_fsm = server_fsm_new(main_loop);
  socketloop_set_data(main_loop, server_fsm);

  tcp_ok = listen_tcp(main_loop);
  unix_ok = listen_unix(main_loop);
  if (!tcp_ok && !unix_ok)
    fatal("Could not establish listening sockets");
  socketloop_run(main_loop);
  socketloop_close_listeners(main_loop);
  if (unix_ok)
    unlink("/tmp/management-game");

  signal(SIGINT, SIG_DFL);
  signal(SIGHUP, SIG_DFL);
  signal(SIGTERM, SIG_DFL);
  
  socketloop_delete(main_loop);
  server_fsm_delete(server_fsm);
  message("Server shutdown");
  return 0;
}



static int listen_tcp(SocketLoop *loop)
{
  struct sockaddr_in addr;
  int sock;

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock<0)
  {
    warning("Could not create a listening tcp socket: %s", strerror(errno));
    return 0;
  }
  memset(&addr, sizeof(addr), 0);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(TCP_IN_PORT);
  addr.sin_addr.s_addr = INADDR_ANY;
  if (bind(sock, (struct sockaddr *)&addr, sizeof(addr))<0)
  {
    warning("Could not bind listening tcp socket: %s", strerror(errno));
    return 0;
  }
  message("Listening on tcp port %d", TCP_IN_PORT);
  socketloop_listen(loop, sock);
  return 1;
}


static int listen_unix(SocketLoop *loop)
{
  struct sockaddr_un addr;
  int sock;

  sock = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sock<0)
  {
    warning("Could not create a listening unix socket: %s", strerror(errno));
    return 0;
  }
  memset(&addr, sizeof(addr), 0);
  addr.sun_family = AF_UNIX;
  strcpy(addr.sun_path, "/tmp/management-game");
  if (bind(sock, (struct sockaddr *)&addr, sizeof(addr))<0)
  {
    warning("Could not bind listening unix socket: %s", strerror(errno));
    return 0;
  }
  message("Listening on unix socket %s", addr.sun_path);
  socketloop_listen(loop, sock);
  return 1;
}

/* Signal handler */

static void terminate_handler(int sig)
{
  socketloop_stop(main_loop);
  signal(sig, terminate_handler);
}


/* Socket event handlers */

static void on_client_connect(SocketLoop *loop, int fd)
{
  FSMEvent event;
  event.type = EV_CONNECT;
  event.fd = fd;
  event.command = NULL;
  event.command_args = NULL;
  fsm_event((FSM *) socketloop_get_data(loop), &event);
}


static void on_incoming_message(SocketLoop *loop, int fd, const char *msg)
{
  TokenList *tl;
  tl = lexer_split(msg);
  if (tl == NULL)
  {
    trace("Bad message from %d", fd);
    socketloop_send(loop, fd, "error \"Bad syntax\"");
    socketloop_drop_client(loop, fd);
  }
  else if (tl->tokens != NULL)
  {
    const char *command = list_head(tl->tokens, char *);
    List args = tl->tokens->next;
    FSMEvent event;
    event.type = EV_COMMAND;
    event.fd = fd;
    event.command = command;
    event.command_args = args;
    
    fsm_event((FSM *) socketloop_get_data(loop), &event);
  }
  lexer_delete(tl);
}


static void on_client_disconnect(SocketLoop *loop, int fd)
{
  FSMEvent event;
  event.type = EV_DISCONNECT;
  event.fd = fd;
  event.command = NULL;
  event.command_args = NULL;
  fsm_event((FSM *) socketloop_get_data(loop), &event);
}
