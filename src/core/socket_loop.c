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


#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>         
#include <sys/socket.h>
#include <sys/select.h>

#include "core/socket_loop.h"
#include "core/list.h"
#include "core/buffer.h"
#include "core/log.h"
#include "core/smq.h"

/* Socket read buffer size */
#define READBUFSIZE 1024

struct SocketLoop
{
  /* Listening sockets */
  List *listeners;
  
  /* Connection sockets */
  List *clients;

  /* A flag whether to continue looping */
  volatile sig_atomic_t do_continue;

  /* Event handler callbacks */
  const SocketLoopEventHandler *handler;

  /* Some user-supplied data */
  void *user_data;
};

typedef struct SocketLoopClient
{
  SocketLoop *owner;

  /* Client connection descriptor */
  int fd;

  /* Read buffer */
  Buffer *buffer;

  /* Send buffer */
  SocketMessageQueue *smq;

  /* Flags */
  /* Dead connection: will be deleted */
  unsigned int is_active:1; 
  /* About to close connection; becomes dead after buffer is flushed */
  unsigned int is_dropped:1; 
} SocketLoopClient;


static void fill_fd_set(SocketLoop *loop, fd_set *readfds, 
    fd_set *writefds, int *pmaxfd);
static void check_events(SocketLoop *loop, fd_set *fds_read, 
    fd_set *fds_send, fd_set *fds_except);
static void accept_connection(SocketLoop *loop, int listener_fd);
static void read_data(SocketLoop *loop, SocketLoopClient *client);
static void delete_client(SocketLoopClient *client);
static SocketLoopClient *find_client(SocketLoop *loop, int fd);



SocketLoop *socketloop_new(const SocketLoopEventHandler *handler)
{
  SocketLoop *loop = (SocketLoop *) calloc(sizeof(SocketLoop), 1);
  loop->handler = handler;
  return loop;
}


void socketloop_delete(SocketLoop *loop)
{
  assert(loop != NULL);
  free(loop);
}


void socketloop_listen(SocketLoop *loop, int fd)
{
  if (listen(fd, 5) == 0)
    loop->listeners = list_push(loop->listeners, int, fd);
  else
    warning("Failed to listen on %d: %s", fd, strerror(errno));
}


void socketloop_close_listeners(SocketLoop *loop)
{
  FOREACH(int, fd, loop->listeners)
  {
    if (0 != close(fd))
      warning("Failed to close %d: %s", fd, strerror(errno));
  } FOREACH_END;
  list_delete(loop->listeners);
  loop->listeners = NULL;
}


void socketloop_run(SocketLoop *loop)
{
  loop->do_continue = 1;
  while (loop->do_continue)
  {
    fd_set fds_read;
    fd_set fds_except;
    fd_set fds_send;
    int maxfd;
    int retval;

    /* Set up fd's for select */
    fill_fd_set(loop, &fds_read, &fds_send, &maxfd);
    memcpy(&fds_except, &fds_read, sizeof(fd_set));

    retval = select(maxfd+1, &fds_read, &fds_send, &fds_except, NULL);
    if (retval != -1)
      check_events(loop, &fds_read, &fds_send, &fds_except);
    else
    {
      if (errno == EINTR)
      {
        trace("Select interrupted by signal");
        continue;
      }
      else
      {
        warning("Select failed: %s", strerror(errno));
        break;
      }
    }
  }
}


void socketloop_stop(SocketLoop *loop)
{
  loop->do_continue = 0;
}


void socketloop_send(SocketLoop *loop, int client_fd, const char *command)
{
  SocketLoopClient *client = find_client(loop, client_fd);
  smq_enqueue(client->smq, command);
  smq_try_send(client->smq, client->fd);
}


void socketloop_add_client(SocketLoop *loop, int fd)
{
  SocketLoopClient *client = (SocketLoopClient *) malloc(sizeof(SocketLoopClient));

  client->owner = loop;
  client->fd = fd;
  client->buffer = buffer_new();
  client->smq = smq_new();
  client->is_active = 1;
  client->is_dropped = 0;
  
  loop->clients = list_push(loop->clients, SocketLoopClient *, client);

  loop->handler->on_client_connect(loop, fd);
}


void socketloop_drop_client(SocketLoop *loop, int client_fd)
{
  SocketLoopClient *client = find_client(loop, client_fd);
  client->is_dropped = 1;
  if (-1 == shutdown(client->fd, SHUT_RD))
    warning("Failed to shutdown recv on %d: %s", client->fd, strerror(errno));
}


void socketloop_set_data(SocketLoop *loop, void *data)
{
  loop->user_data = data;
}


void *socketloop_get_data(SocketLoop *loop)
{
  return loop->user_data;
}



/** 
 * Internal subroutines 
 */

static void fill_fd_set(SocketLoop *loop, fd_set *readfds, 
    fd_set *writefds, int *pmaxfd)
{
  int maxfd = 0;
  FD_ZERO(readfds);
  FD_ZERO(writefds);

  FOREACH(int, fd, loop->listeners)
  {
    FD_SET(fd, readfds);
    if (fd > maxfd)
      maxfd = fd;
  } FOREACH_END;

  FOREACH(SocketLoopClient *, client, loop->clients)
  {
    FD_SET(client->fd, readfds);
    if (!smq_is_empty(client->smq))
      FD_SET(client->fd, writefds);
    if (client->fd > maxfd)
      maxfd = client->fd;
  } FOREACH_END;

  *pmaxfd = maxfd;
}


static int client_is_dead(ListItem item)
{
  SocketLoopClient *client = (SocketLoopClient *) item;
  return !client->is_active 
    || (client->is_dropped && smq_is_empty(client->smq));
}
static void client_destr(ListItem item)
{
  delete_client((SocketLoopClient *) item);
}
static void check_events(SocketLoop *loop, fd_set *fds_read,
    fd_set *fds_send, fd_set *fds_except)
{
  FOREACH(int, fd, loop->listeners)
  {
    if (FD_ISSET(fd, fds_except))
      warning("Error condition on listening socket %d", fd);
    if (FD_ISSET(fd, fds_read))
      accept_connection(loop, fd);
  } FOREACH_END;

  FOREACH(SocketLoopClient *, client, loop->clients)
  {
    if (FD_ISSET(client->fd, fds_except))
      warning("Error condition on client socket %d", client->fd);
    if (FD_ISSET(client->fd, fds_read))
      read_data(loop, client);
    if (FD_ISSET(client->fd, fds_send))
      smq_try_send(client->smq, client->fd);
  } FOREACH_END;

  /* delete dead clients */
  loop->clients = list_filter(loop->clients, SocketLoopClient *, 
      client_is_dead, client_destr);
}


static void accept_connection(SocketLoop *loop, int listener_fd)
{
  int fd = accept(listener_fd, NULL, NULL);
  if (fd < 0)
    warning("Failed to accept a client connection: %s", strerror(errno));
  else
    socketloop_add_client(loop, fd);
}

static void read_data(SocketLoop *loop, SocketLoopClient *client)
{
  unsigned char readbuf[READBUFSIZE];
  ssize_t retval;
  while ((retval = recv(client->fd, readbuf, READBUFSIZE, MSG_DONTWAIT)) > 0)
  {
    ssize_t i;
    for (i=0; i<retval; i++)
    {
      if (readbuf[i]=='\n')
      {
        loop->handler->on_incoming_message(loop, client->fd, client->buffer->c_str);
        buffer_clear(client->buffer);
      }
      else
        buffer_putchar(client->buffer, readbuf[i]);
    }
  }
  if (retval==0)
  {
    /* Connection closed gracefully */
    client->is_active = 0;
  }
  else if (errno != EAGAIN && errno != EWOULDBLOCK)
    warning("Client %d: recv error: %s", client->fd, strerror(errno));
}


static void delete_client(SocketLoopClient *client)
{
  SocketLoop *loop = client->owner;
  loop->handler->on_client_disconnect(loop, client->fd);

  buffer_delete(client->buffer);
  smq_delete(client->smq);

  close(client->fd);
  free(client);
}

static SocketLoopClient *find_client(SocketLoop *loop, int fd)
{
  FOREACH(SocketLoopClient *, client, loop->clients)
  {
    if (client->fd==fd)
      return client;
  } FOREACH_END;

  fatal("socket_loop: Nonexistent client requested: %d", fd);
  return NULL;
}


