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

#include "core/log.h"
#include "server/commands.h"
#include "server/server_fsm.h"

/* Max user's display name length */
#define MAXNAMELENGTH 30

static void cmd_identify(ServerData *d, ClientData *client, List args);
static void cmd_quit(ServerData *d, ClientData *client, List args);
static void cmd_ready(ServerData *d, ClientData *client, List args);
static void cmd_notready(ServerData *d, ClientData *client, List args);
static void cmd_lslobby(ServerData *d, ClientData *client, List args);
static void cmd_lsgame(ServerData *d, ClientData *client, List args);
static void cmd_start(ServerData *d, ClientData *client, List args);
static void cmd_step(ServerData *d, ClientData *client, List args);
static void cmd_pause(ServerData *d, ClientData *client, List args);
static void cmd_abort(ServerData *d, ClientData *client, List args);
static void cmd_run(ServerData *d, ClientData *client, List args);
static void cmd_buy(ServerData *d, ClientData *client, List args);
static void cmd_sell(ServerData *d, ClientData *client, List args);
static void cmd_produce(ServerData *d, ClientData *client, List args);
static void cmd_build(ServerData *d, ClientData *client, List args);

static void send_bad_command(ServerData *d, int fd);
static void send_ack(ServerData *d, int fd);


/* A convenience macro to declare commands */
#define MKCOMMAND(_nargs, _cmd, _states) {cmd_##_cmd, _nargs, #_cmd, _states}
/* A shorthand for the set of all "normal" states */
#define CL_VALID (CL_SUPERVISOR \
    | CL_IN_LOBBY | CL_IN_LOBBY_ACK \
    | CL_IN_GAME | CL_IN_GAME_WAIT)
const struct CommandDescription
{
  void (*handler)(ServerData *d, ClientData *client, List args);
  int n_args;
  const char *str;
  enum ClientState allowed_states;
} commands[] = 
{
  /* Login command */
  MKCOMMAND(2, identify,    CL_CONNECTED),

  /* Readiness state. Both are valid in lobby, 'ready' is available in-game */
  MKCOMMAND(0, ready,       CL_IN_LOBBY | CL_IN_GAME),
  MKCOMMAND(0, notready,    CL_IN_LOBBY_ACK),

  /* General information commands. Can be used at any moment of time */
  MKCOMMAND(0, quit,        CL_VALID),
  MKCOMMAND(0, lslobby,     CL_VALID),
  MKCOMMAND(0, lsgame,      CL_VALID),

  /* Game flow control */
  MKCOMMAND(0, start,       CL_SUPERVISOR),
  MKCOMMAND(0, step,        CL_SUPERVISOR),
  MKCOMMAND(0, run,         CL_SUPERVISOR),
  MKCOMMAND(0, pause,       CL_SUPERVISOR),
  MKCOMMAND(0, abort,       CL_SUPERVISOR),

  /* Im-game commands */
  MKCOMMAND(0, buy,         CL_IN_GAME),
  MKCOMMAND(0, sell,        CL_IN_GAME),
  MKCOMMAND(0, build,       CL_IN_GAME),
  MKCOMMAND(0, produce,     CL_IN_GAME)
};
const unsigned int n_commands = sizeof(commands)/sizeof(struct CommandDescription);
#undef MKCOMMAND


void command_exec(ServerData *d, ClientData *client, 
    const char *cmd_str, List cmd_args)
{
  unsigned int i;
  for (i=0; i<n_commands; i++)
    if (strcmp(cmd_str, commands[i].str) == 0)
    {
      if ((commands[i].allowed_states & client->state)
          && commands[i].n_args == list_size(cmd_args) )
      {
        commands[i].handler(d, client, cmd_args);
      }
      else
        server_send_message(d, client->fd,
            "error \"Command not available right now\"");
      return;
    }

  send_bad_command(d, client->fd); 
}

static void send_bad_command(ServerData *d, int fd)
{
  server_send_message(d, fd,
      "error \"Bad command\"");
}

static void send_ack(ServerData *d, int fd)
{
  server_send_message(d, fd,
      "ack");
}


/* 
 * Commands' implementation
 */

static void cmd_identify(ServerData *d, ClientData *client, List args)
{
  char *type;
  char *name;
  type = list_head(args, char *);
  name = list_head(args->next, char *);

  if (strlen(name)>MAXNAMELENGTH)
  {
    server_send_message(d, client->fd, 
        "error \"Name too long.\"");
    server_send_message(d, client->fd,
        "message Server \"The length of your name must not exceed %d characters\"",
        MAXNAMELENGTH); 
  } 
  else if (strcmp(type, "client") == 0)
  {
    client->state = CL_IN_LOBBY;
    client->name = strdup(name);
    send_ack(d, client->fd); 
    server_send_message(d, client->fd,
        "message Server \"Welcome to the lobby, %.100s\"",
        client->name); 
    message("%s has entered the lobby.", client->name);
  }
  else if (strcmp(type, "supervisor") == 0)
  {
    client->state = CL_SUPERVISOR;
    client->name = strdup(name);
    send_ack(d, client->fd);
    server_send_message(d, client->fd,
        "message Server \"Hi, %.100s, take your seat. I'm at your service.\"",
        client->name); 
    message("%s has connected as a supervisor", client->name);
  }
  else
    send_bad_command(d, client->fd);
}


static void cmd_quit(ServerData *d, ClientData *client, List args)
{
  send_ack(d, client->fd);
  server_send_message(d, client->fd, 
      "message Server \"See ya!\""); 
  socketloop_drop_client(d->loop, client->fd);
  /* Client will get disconnected somewhat later */
}


static void cmd_ready(ServerData *d, ClientData *client, List args)
{
  if (client->state == CL_IN_LOBBY)
  {
    send_ack(d, client->fd);
    client->state = CL_IN_LOBBY_ACK;
    server_send_message(d, client->fd, 
        "message Server \"You are ready to play. "
        "Please wait for the game to start\""); 
    message("%s is ready to play.", client->name);
  }
  else if (client->state == CL_IN_GAME)
  {
    send_ack(d, client->fd);
    client->state = CL_IN_GAME_WAIT;
    server_send_message(d, client->fd,
        "message Server \"Your turn is over. "
        "Please wait for other players to get ready.\"");
    message("%s has finished his turn.", client->name);
    d->n_waitfor--;
  }
}


static void cmd_notready(ServerData *d, ClientData *client, List args)
{
  send_ack(d, client->fd);
  client->state = CL_IN_LOBBY;
  server_send_message(d, client->fd, 
      "message Server \"You are not ready to play.\""); 
  message("%s is not ready to play.", client->name);
}


static void cmd_lslobby(ServerData *d, ClientData *client, List args)
{
  server_send_message(d, client->fd, 
      "lslobby begin");
  FOREACH(ClientData *, item, d->clients)
  {
    if (item->state & (CL_IN_LOBBY | CL_IN_LOBBY_ACK))
      server_send_message(d, client->fd, 
          "lslobby item %c \"%s\"",
          item->state==CL_IN_LOBBY?'-':'+',
          item->name);
  } FOREACH_END;
  server_send_message(d, client->fd, 
      "lslobby end");
}


static void cmd_lsgame(ServerData *d, ClientData *client, List args)
{
  server_send_message(d, client->fd, 
      "lsgame begin");
  FOREACH(ClientData *, item, d->clients)
  {
    if (item->state & (CL_IN_GAME | CL_IN_GAME_WAIT))
      server_send_message(d, client->fd, 
          "lsgame item %c \"%s\"",
          item->state==CL_IN_GAME?'-':'+',
          item->name);
  } FOREACH_END;
  server_send_message(d, client->fd,
      "lsgame end");
}


static void cmd_start(ServerData *d, ClientData *client, List args)
{
  if (d->fsm->state != ST_LOBBY)
  {
    send_bad_command(d, client->fd);
    return;
  }

  send_ack(d, client->fd);
  fsm_set_next_state(d->fsm, ST_BEFORE_ROUND);
  fsm_finish_loop(d->fsm);
}


static void cmd_step(ServerData *d, ClientData *client, List args)
{
  if (d->fsm->state != ST_BEFORE_ROUND)
  {
    send_bad_command(d, client->fd);
    return;
  }

  send_ack(d, client->fd);
  fsm_set_next_state(d->fsm, ST_ROUND);
  fsm_finish_loop(d->fsm);
}


static void cmd_run(ServerData *d, ClientData *client, List args)
{
  if (d->fsm->state != ST_BEFORE_ROUND)
  {
    send_bad_command(d, client->fd);
    return;
  }

  send_ack(d, client->fd);
  d->continuous_game = 1;
  fsm_set_next_state(d->fsm, ST_ROUND);
  fsm_finish_loop(d->fsm);
}


static void cmd_pause(ServerData *d, ClientData *client, List args)
{
  if (args != NULL || d->fsm->state != ST_ROUND)
  {
    send_bad_command(d, client->fd);
    return;
  }

  send_ack(d, client->fd);
  d->continuous_game = 0;
}


static void cmd_abort(ServerData *d, ClientData *client, List args)
{
  if (d->fsm->state != ST_ROUND && d->fsm->state != ST_BEFORE_ROUND)
  {
    send_bad_command(d, client->fd);
    return;
  }

  send_ack(d, client->fd);
  d->abort_game = 1;
  fsm_set_next_state(d->fsm, ST_LOBBY);
  fsm_finish_loop(d->fsm);
}


static void cmd_buy(ServerData *d, ClientData *client, List args)
{
  send_ack(d, client->fd);
  server_send_message(d, client->fd, 
      "You wanna buy?!");
}


static void cmd_sell(ServerData *d, ClientData *client, List args)
{
  send_ack(d, client->fd);
  server_send_message(d, client->fd, 
      "You wanna sell?!");
}


static void cmd_produce(ServerData *d, ClientData *client, List args)
{
  send_ack(d, client->fd);
  server_send_message(d, client->fd, 
      "You wanna produce?!");
}


static void cmd_build(ServerData *d, ClientData *client, List args)
{
  send_ack(d, client->fd);
  server_send_message(d, client->fd, 
      "You wanna build?!");
}


