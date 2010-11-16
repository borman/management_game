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



static ClientData *new_client(int fd);
static ClientData *find_client(ServerData *d, int fd);
static void delete_client(ClientData *client); 

static void accept_client(ServerData *d, int fd);
static void drop_client(ServerData *d, int fd);

static void lobby_on_enter(FSM *fsm);
static void lobby_on_exit(FSM *fsm);

static void round_on_enter(FSM *fsm);
static void round_on_exit(FSM *fsm);

static void before_round_on_enter(FSM *fsm);
static void before_round_on_exit(FSM *fsm);

static void on_event(FSM *fsm, FSMEvent *event);
static void on_command(FSM *fsm, FSMEvent *event);

static const struct FSMState server_states[] = 
{
  {
    "Lobby",
    lobby_on_enter,
    on_event,
    lobby_on_exit
  },
  {
    "Before round",
    before_round_on_enter,
    on_event,
    before_round_on_exit
  },
  {
    "Round",
    round_on_enter,
    on_event,
    round_on_exit
  }
};

static const FSM server_dummy = 
{
  "ManagementServer",
  sizeof(server_states)/sizeof(struct FSMState),
  server_states,
  NULL, 0, 0, 0
};


/**
 * Init/destroy
 */

FSM *server_fsm_new(SocketLoop *loop)
{
  FSM *fsm = (FSM *) malloc(sizeof(FSM));
  ServerData *d = (ServerData *) malloc(sizeof(ServerData));

  memcpy(fsm, &server_dummy, sizeof(FSM));
  d->fsm = fsm;
  d->loop = loop;
  d->clients = NULL;
  d->continuous_game = 0;
  d->abort_game = 0;
  d->round_counter = 0;
  fsm->data = d;
  
  fsm_init(fsm, ST_LOBBY);
  return fsm;
}


void server_fsm_delete(FSM *fsm)
{
  free(fsm->data);
  free(fsm);
}



/**
 * Event handlers
 */

/* -- Common */

static void on_event(FSM *fsm, FSMEvent *event)
{
  ServerData *d = (ServerData *) fsm->data;
  switch (event->type)
  {
    case EV_CONNECT:
      accept_client(d, event->fd);
      break;
    case EV_DISCONNECT:
      /* delete client %d */
      drop_client(d, event->fd);
      break;
    case EV_COMMAND:
      on_command(fsm, event);
      break;
  }
}


static void on_command(FSM *fsm, FSMEvent *event)
{
  ServerData *d = (ServerData *) fsm->data;
  ClientData *client = find_client(d, event->fd);

  trace("%s: Command from %d/%#x: %s +%d", 
      fsm->states[fsm->state].name,
      client->fd, client->state, 
      event->command, list_size(event->command_args));

  command_exec(d, client, event->command, event->command_args);
}


/* -- Lobby */

static void lobby_on_enter(FSM *fsm)
{
  (void)fsm;
  /*
  ServerData *d = (ServerData *) fsm->data;
  */
}


static void lobby_on_exit(FSM *fsm)
{
  ServerData *d = (ServerData *) fsm->data;
  
  message("The game begins.");
  d->round_counter = 0;
  FOREACH(ClientData *, client, d->clients)
  {
    if (client->state == CL_IN_LOBBY_ACK)
    {
      client->state = CL_IN_GAME;
      server_send_message(d, client->fd, 
          "game_start");
    }
  } FOREACH_END;
}


/* --- Before round */

static void before_round_on_enter(FSM *fsm)
{
  ServerData *d = (ServerData *) fsm->data;
  message("Round %d begins.", d->round_counter);
  if (d->continuous_game)
  {
    fsm_finish_loop(fsm);
    fsm_set_next_state(fsm, ST_ROUND);
  }
}

static void before_round_on_exit(FSM *fsm)
{
  ServerData *d = (ServerData *) fsm->data;
  if (d->abort_game)
    return; /* TODO */

  FOREACH(ClientData *, client, d->clients)
  {
    if (client->state == CL_IN_GAME)
      server_send_message(d, client->fd,
        "round_start %u", 
        d->round_counter);
  } FOREACH_END;
}


/* -- Round */

static void round_on_enter(FSM *fsm)
{
  (void)fsm;
  /*
  ServerData *d = (ServerData *) fsm->data;
  */
}

static void round_on_exit(FSM *fsm)
{
  (void)fsm;
  /*
  ServerData *d = (ServerData *) fsm->data;
  */
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
  if (client->name)
    free(client->name);
  free(client);
}


static void accept_client(ServerData *d, int fd)
{
  /* new client */
  d->clients = list_push(d->clients, ClientData *, new_client(fd));
  server_send_message(d, fd, 
      "message Server \"Hello there! Please, identify yourself.\"");
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


void server_send_message(ServerData *d, int client, const char *format, ...)
{
  va_list args;
  char buf[MAXREPLYLENGTH];
  va_start(args, format);
  vsnprintf(buf, MAXREPLYLENGTH, format, args);
  va_end(args);
  socketloop_send(d->loop, client, buf);
}

