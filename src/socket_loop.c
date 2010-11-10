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

#include "socket_loop.h"
#include "list.h"
#include "buffer.h"
#include "debug.h"
#include "events.h"
#include "smq.h"

/* Socket read buffer size */
#define READBUFSIZE 1024

struct SocketLoop
{
  List listeners;
  List clients;
  volatile sig_atomic_t do_continue;
};

typedef struct SocketLoopClient
{
  /* Client connection descriptor */
  int fd;

  /* Read buffer */
  Buffer *buffer;

  /* Send buffer */
  SocketMessageQueue *smq;

  /* Flags */
  /* Dead connection: will be deleted */
  int is_active:1; 
  /* About to close connection; becomes dead after buffer is flushed */
  int is_dropped:1; 
} SocketLoopClient;


static void fill_fd_set(SocketLoop *loop, fd_set *readfds, 
    fd_set *writefds, int *pmaxfd);
static void check_events(SocketLoop *loop, fd_set *fds_read, 
    fd_set *fds_send, fd_set *fds_except);
static void accept_connection(SocketLoop *loop, int listener_fd);
static void read_data(SocketLoop *loop, SocketLoopClient *client);
static void add_client(SocketLoop *loop, int fd);
static void delete_client(SocketLoop *loop, SocketLoopClient *client);
static SocketLoopClient *find_client(SocketLoop *loop, int fd);



SocketLoop *socketloop_new()
{
  SocketLoop *loop = (SocketLoop *) calloc(sizeof(SocketLoop), 1);
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
  } FOREACH_END
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
  /* TODO */
  SocketLoopClient *client = find_client(loop, client_fd);
  trace("Send %d <- %s", client_fd, command);
  smq_enqueue(client->smq, command);
  smq_try_send(client->smq, client->fd);
}


void socketloop_drop_client(SocketLoop *loop, int client)
{
  /* TODO */
  trace("Drop client %d", client);
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
  } FOREACH_END

  FOREACH(SocketLoopClient *, client, loop->clients)
  {
    FD_SET(client->fd, readfds);
    if (!smq_is_empty(client->smq))
      FD_SET(client->fd, writefds);
    if (client->fd > maxfd)
      maxfd = client->fd;
  } FOREACH_END

  *pmaxfd = maxfd;
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
  } FOREACH_END

  FOREACH(SocketLoopClient *, client, loop->clients)
  {
    if (FD_ISSET(client->fd, fds_except))
      warning("Error condition on client socket %d", client->fd);
    if (FD_ISSET(client->fd, fds_read))
      read_data(loop, client);
    if (FD_ISSET(client->fd, fds_send))
      smq_try_send(client->smq, client->fd);
  } FOREACH_END

  /* delete dead clients */
  /* TODO: make a list filter macro */
  {
    List l = loop->clients;
    List alive = NULL;
    while (l != NULL)
    {
      SocketLoopClient *client = list_head(l, SocketLoopClient *);
      if (!client->is_active)
        delete_client(loop, client);
      else
        alive = list_push(alive, SocketLoopClient *, client);
      l = list_pop(l);
    }
    loop->clients = alive;
  }
}


static void accept_connection(SocketLoop *loop, int listener_fd)
{
  int fd = accept(listener_fd, NULL, NULL);
  if (fd < 0)
    warning("Failed to accept a client connection: %s", strerror(errno));
  else
    add_client(loop, fd);
}

static void read_data(SocketLoop *loop, SocketLoopClient *client)
{
  unsigned char readbuf[READBUFSIZE];
  ssize_t retval;
  while ((retval = recv(client->fd, readbuf, READBUFSIZE, MSG_DONTWAIT)) > 0)
  {
    size_t i;
    for (i=0; i<retval; i++)
    {
      if (readbuf[i]=='\n')
      {
        event_client_incoming_message(loop, client->fd, client->buffer->c_str);
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


static void add_client(SocketLoop *loop, int fd)
{
  SocketLoopClient *client = (SocketLoopClient *) malloc(sizeof(SocketLoopClient));

  client->fd = fd;

  client->buffer = buffer_new();

  client->smq = smq_new();

  client->is_active = 1;
  client->is_dropped = 0;
  
  loop->clients = list_push(loop->clients, SocketLoopClient *, client);

  event_client_connected(loop, fd);
}


static void delete_client(SocketLoop *loop, SocketLoopClient *client)
{
  event_client_disconnected(loop, client->fd);

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
  } FOREACH_END

  fatal("Nonexistent client %d requested", fd);
  return NULL;
}


