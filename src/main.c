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

#include <signal.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "socket_loop.h"
#include "debug.h"
#include "server_fsm.h"

#define TCP_IN_PORT 8982

static void listen_tcp(SocketLoop *loop);
static void terminate_handler(int);


SocketLoop *main_loop;


int main(int argc, char **argv)
{
  FSM *server_fsm;

  message("Server started");
  main_loop = socketloop_new();

  signal(SIGINT, terminate_handler);
  signal(SIGHUP, terminate_handler);
  signal(SIGTERM, terminate_handler);
  
  server_fsm = server_fsm_new(main_loop);
  socketloop_set_data(main_loop, server_fsm);

  listen_tcp(main_loop);
  socketloop_run(main_loop);
  socketloop_close_listeners(main_loop);

  signal(SIGINT, SIG_DFL);
  signal(SIGHUP, SIG_DFL);
  signal(SIGTERM, SIG_DFL);
  
  socketloop_delete(main_loop);
  server_fsm_delete(server_fsm);
  message("Server shutdown");
  return 0;
}



static void listen_tcp(SocketLoop *loop)
{
  struct sockaddr_in addr;
  int sock;

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock<0)
  {
    warning("Could not create a listening tcp socket: %s", strerror(errno));
    return;
  }
  memset(&addr, sizeof(addr), 0);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(TCP_IN_PORT);
  addr.sin_addr.s_addr = INADDR_ANY;
  if (bind(sock, (struct sockaddr *)&addr, sizeof(addr))<0)
  {
    warning("Could not bind listening tcp socket: %s", strerror(errno));
    return;
  }
  message("Listening on tcp port %d", TCP_IN_PORT);
  socketloop_listen(loop, sock);
}


static void terminate_handler(int sig)
{
  socketloop_stop(main_loop);
  signal(sig, terminate_handler);
}

