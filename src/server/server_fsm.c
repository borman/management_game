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
#include "server/game.h"

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


void server_send_reply(ServerData *d, int fd, const char *format, ...)
{
  va_list args;
  char buf[MAXREPLYLENGTH];
  va_start(args, format);
  vsnprintf(buf, MAXREPLYLENGTH, format, args);
  va_end(args);
  socketloop_send(d->loop, fd, buf);
}

void server_send_broadcast(ServerData *d, int client_mask, const char *format, ...)
{
  va_list args;
  char buf[MAXREPLYLENGTH];
  strcpy(buf, "+ ");
  va_start(args, format);
  vsnprintf(buf+2, MAXREPLYLENGTH-2, format, args);
  va_end(args);
  FOREACH(ClientData *, client, d->clients)
  {
    if (client->state & client_mask)
      socketloop_send(d->loop, client->fd, buf);
  } FOREACH_END;
}

void server_send_message(ServerData *d, int fd, 
    const char *format, ...) 
{
  va_list args;
  char buf[MAXREPLYLENGTH];
  char format_buf[MAXREPLYLENGTH];
  snprintf(format_buf, MAXREPLYLENGTH, "> Server \"%s\"", format);
  va_start(args, format);
  vsnprintf(buf, MAXREPLYLENGTH, format_buf, args);
  va_end(args);
  socketloop_send(d->loop, fd, buf);
}

void server_send_log_message_v(ServerData *d, 
    const char *format, va_list args)
{
  char buf[MAXREPLYLENGTH];
  char format_buf[MAXREPLYLENGTH];
  snprintf(format_buf, MAXREPLYLENGTH, "> Server \"%s\"", format);
  vsnprintf(buf, MAXREPLYLENGTH, format_buf, args);
  FOREACH(ClientData *, client, d->clients)
  {
    if (client->state & CL_AUTHENTICATED)
      socketloop_send(d->loop, client->fd, buf);
  } FOREACH_END;
}

void server_set_client_state(ServerData *d, ClientData *client, 
    enum ClientState state)
{
  const char *state_str;
  client->state = state;
  switch (state)
  {
    case CL_CONNECTED:
      state_str = "auth";
      break;
    case CL_SUPERVISOR:
      state_str = "super";
      break;
    case CL_IN_LOBBY:
      state_str = "lobby";
      break;
    case CL_IN_LOBBY_ACK:
      state_str = "lobby_ready";
      break;
    case CL_IN_GAME:
      state_str = "game_active";
      break;
    case CL_IN_GAME_WAIT:
      state_str = "game";
      break;
    default:
      state_str = NULL;
  }
  if (state_str != NULL)
    server_send_reply(d, client->fd, "$ %s", state_str);
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
      if (fsm->state == ST_ROUND || fsm->state == ST_BEFORE_ROUND)
        game_check_players(d);
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
  d->n_players = 0;
  d->continuous_game = 0;

  /* Update client states */
  FOREACH(ClientData *, client, d->clients)
  {
    if (client->state == CL_IN_LOBBY_ACK)
    {
      server_set_client_state(d, client, CL_IN_GAME_WAIT);
      d->n_players++;
    }
  } FOREACH_END;

  game_start(d);
}


/* --- Before round */

static void before_round_on_enter(FSM *fsm)
{
  ServerData *d = (ServerData *) fsm->data;
  if (d->continuous_game)
    fsm_switch_state(fsm, ST_ROUND);
}

static void before_round_on_exit(FSM *fsm)
{
}


/* -- Round */

static void round_on_enter(FSM *fsm)
{
  ServerData *d = (ServerData *) fsm->data;
  message("Round %d begins.", d->round_counter);
  game_start_round(d);

  /* Update client states */
  FOREACH(ClientData *, client, d->clients)
  {
    if (client->state == CL_IN_GAME_WAIT)
      server_set_client_state(d, client, CL_IN_GAME);
  } FOREACH_END;
  d->n_waitfor = d->n_players;
}

static void round_on_exit(FSM *fsm)
{
  ServerData *d = (ServerData *) fsm->data;
  game_finish_round(d);
  d->round_counter++;
}



/**
 * Utility subroutines
 */

static ClientData *new_client(int fd)
{
  ClientData *client = (ClientData *) calloc(1, sizeof(ClientData));
  client->fd = fd;
  client->name = NULL;
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
  ClientData *client = new_client(fd);
  server_set_client_state(d, client, CL_CONNECTED);
  d->clients = list_push(d->clients, ClientData *, client);
  server_send_message(d, fd, "Hello there! Please, identify yourself.");
}

static int client_is_dead(ListItem item)
{
  ClientData *client = (ClientData *) item;
  return client->state == CL_DEAD;
}
static void client_destr(ListItem item)
{
  delete_client((ClientData *) item);
}
static void drop_client(ServerData *d, int fd)
{
  ClientData *client = find_client(d, fd);

  if (client->name != NULL)
  {
   if (client->state == CL_SUPERVISOR)
     message("Supervisor %s has disconnected", client->name);
   else if (client->state & (CL_IN_LOBBY | CL_IN_LOBBY_ACK))
     message("%s has left the lobby.", client->name);
   else if (client->state & (CL_IN_GAME | CL_IN_GAME_WAIT))
   {
     message("%s has left the game.", client->name);
     game_remove_player(d, client);
   }
  }

  server_set_client_state(d, client, CL_DEAD);
  d->clients = list_filter(d->clients, ClientData *,
      client_is_dead, client_destr);
}
