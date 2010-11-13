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


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "core/log.h"
#include "server/server_fsm.h"
#include "server/commands.h"

#define MAXREPLYLENGTH 256
#define MAXNAMELENGTH 30

enum ServerState
{
  ST_LOBBY = 0,
  ST_ROUND
};

typedef struct ClientData
{
  int fd;
  enum ClientState state;
  char *name;
} ClientData;

typedef struct ServerData
{
  SocketLoop *loop;
  List clients; /* List<ClientData *> */
} ServerData;



static ClientData *new_client(int fd);
static ClientData *find_client(ServerData *d, int fd);
static void delete_client(ClientData *client); 
static void drop_client(ServerData *d, int fd);
static void send_message(ServerData *d, int client, const char *format, ...);

static void lobby_on_enter(FSM *fsm);
static void lobby_on_event(FSM *fsm, FSMEvent *event);
static void lobby_on_command(FSM *fsm, FSMEvent *event);
static void lobby_on_exit(FSM *fsm);
static void round_on_enter(FSM *fsm);
static void round_on_event(FSM *fsm, FSMEvent *event);
static void round_on_exit(FSM *fsm);



static const struct FSMState server_states[] = 
{
  {
    "Lobby",
    lobby_on_enter,
    lobby_on_event,
    lobby_on_exit
  },
  {
    "Round",
    round_on_enter,
    round_on_event,
    round_on_exit
  }
};
static const unsigned int n_server_states = 
  sizeof(server_states)/sizeof(struct FSMState);

static const FSM server_dummy = 
{
  "ManagementServer",
  n_server_states,
  server_states
};


/**
 * Init/destroy
 */

FSM *server_fsm_new(SocketLoop *loop)
{
  FSM *fsm = (FSM *) malloc(sizeof(FSM));
  ServerData *d = (ServerData *) malloc(sizeof(ServerData));

  memcpy(fsm, &server_dummy, sizeof(FSM));
  d->loop = loop;
  d->clients = NULL;
  fsm->data = d;
  
  fsm_init(fsm, ST_LOBBY);
  return fsm;
}


void server_fsm_delete(FSM *fsm)
{
  free(fsm);
}



/**
 * Event handlers
 */

static void lobby_on_enter(FSM *fsm)
{
  ServerData *d = (ServerData *) fsm->data;
}


static void lobby_on_event(FSM *fsm, FSMEvent *event)
{
  ServerData *d = (ServerData *) fsm->data;
  switch (event->type)
  {
    case EV_CONNECT:
      /* new client */
      d->clients = list_push(d->clients, ClientData *, new_client(event->fd));
      send_message(d, event->fd, 
          "message Server \"Hello there! Please, identify yourself.\"");
      break;
    case EV_DISCONNECT:
      /* delete client %d */
      drop_client(d, event->fd);
      break;
    case EV_COMMAND:
      lobby_on_command(fsm, event);
      break;
  }
}


static void lobby_on_command(FSM *fsm, FSMEvent *event)
{
  ServerData *d = (ServerData *) fsm->data;
  ClientData *client = find_client(d, event->fd);
  enum Command cmd = command_resolve(client->state, event->command);
  trace("Command from %d/%#x: %s +%d", 
      client->fd, client->state, 
      event->command, list_size(event->command_args));
  switch (cmd)
  {
    case CMD_IDENTIFY:
    {
      char *type;
      char *name;
      if (list_size(event->command_args) != 2)
        goto L_BAD_COMMAND;
      type = list_head(event->command_args, char *);
      name = list_head(event->command_args->next, char *);
      if (strlen(name)>MAXNAMELENGTH)
      {
        send_message(d, client->fd, 
            "auth_fail \"Name too long.\"");
        send_message(d, client->fd,
            "message Server \"The length of your name must not exceed %d characters\"",
            MAXNAMELENGTH); 
      } else if (strcmp(type, "client") == 0)
      {
        client->state = CL_IN_LOBBY;
        client->name = strdup(name);
        send_message(d, client->fd, "auth_ok");
        send_message(d, client->fd,
            "message Server \"Welcome to the lobby, %.100s\"",
            client->name); 
        message("%s has entered the lobby.", client->name);
      }
      else if (strcmp(type, "supervisor") == 0)
      {
        client->state = CL_SUPERVISOR;
        client->name = strdup(name);
        send_message(d, client->fd, "auth_ok");
        send_message(d, client->fd,
            "message Server \"Hi, %.100s, take your seat. I'm at your service.\"",
            client->name); 
        message("%s has connected as a supervisor", client->name);
      }
      else
        goto L_BAD_COMMAND;
    } break;

    case CMD_QUIT:
    {
      if (event->command_args != NULL)
        goto L_BAD_COMMAND;
      socketloop_send(d->loop, client->fd, "message Server \"See ya!\""); 
      socketloop_drop_client(d->loop, client->fd);
    } break;

    default:
      goto L_BAD_COMMAND;
  }
  return;

L_BAD_COMMAND:
  send_message(d, client->fd, "error \"Bad command\""); 
}

static void lobby_on_exit(FSM *fsm)
{
  ServerData *d = (ServerData *) fsm->data;
}


static void round_on_enter(FSM *fsm)
{
  ServerData *d = (ServerData *) fsm->data;
}


static void round_on_event(FSM *fsm, FSMEvent *event)
{
  ServerData *d = (ServerData *) fsm->data;
  switch (event->type)
  {
    case EV_CONNECT:
      socketloop_send(d->loop, event->fd, "error \"The game's on, go away!\"");
      socketloop_drop_client(d->loop, event->fd); 
      break;
    case EV_DISCONNECT:
      /* delete client %d */
      break;
    case EV_COMMAND:
      break;
  }
}


static void round_on_exit(FSM *fsm)
{
  ServerData *d = (ServerData *) fsm->data;
}


/**
 * Utility subroutines
 */

static ClientData *new_client(int fd)
{
  ClientData *client = (ClientData *) malloc(sizeof(ClientData));
  client->fd = fd;
  client->state = CL_CONNECTED;
  return client;
}


static ClientData *find_client(ServerData *d, int fd)
{
  FOREACH(ClientData *, client, d->clients)
  {
    if (client->fd == fd)
      return client;
  } FOREACH_END;
  fatal("server_fsm: Nonexistent client requested: %d", fd);
  return NULL;
}


static void delete_client(ClientData *client)
{
  free(client);
}

static void drop_client(ServerData *d, int fd)
{
  ClientData *client = find_client(d, fd);

  if (client->name != NULL)
  {
   if (client->state == CL_SUPERVISOR)
     message("Supervisor %s has disconnected", client->name);
   else if (client->state == CL_IN_LOBBY || client->state == CL_IN_LOBBY_ACK)
    message("%s has left the lobby.", client->name);
  }

  client->state = CL_DEAD;
  FILTER(d->clients, 
      ClientData *, client, 
      (client->state != CL_DEAD), 
      delete_client(client));
}

static void send_message(ServerData *d, int client, const char *format, ...)
{
  va_list args;
  char buf[MAXREPLYLENGTH];
  va_start(args, format);
  vsnprintf(buf, MAXREPLYLENGTH, format, args);
  va_end(args);
  socketloop_send(d->loop, client, buf);
}
