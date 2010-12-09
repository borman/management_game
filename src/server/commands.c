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
#include "server/game.h"

/* Max user's display name length */
#define MAXNAMELENGTH 30

/**
 * Command handlers return NULL | <static-allocated string>
 * If retval is NULL, result is consedered "ok", otherwise the return value is
 * used as an error message
 */
static const char *do_command_exec(ServerData *d, ClientData *client, 
    const char *cmd_str, List *cmd_args);

/* A convenience macro to declare command handlers */
#define DECLCOMMAND(_name) \
  static const char *cmd_##_name(ServerData *d, ClientData *client, List *args)
/* A convenience macro to declare commands descriptions */
#define MKCOMMAND(_nargs, _argt, _cmd, _states) {cmd_##_cmd, _nargs, _argt, #_cmd, _states}

DECLCOMMAND(auth);
DECLCOMMAND(quit);
DECLCOMMAND(ready);
DECLCOMMAND(notready);
DECLCOMMAND(lslobby);
DECLCOMMAND(lsgame);
DECLCOMMAND(start);
DECLCOMMAND(step);
DECLCOMMAND(pause);
DECLCOMMAND(abort);
DECLCOMMAND(run);
DECLCOMMAND(buy);
DECLCOMMAND(sell);
DECLCOMMAND(produce);
DECLCOMMAND(build);

const struct CommandDescription
{
  const char *(*handler)(ServerData *d, ClientData *client, List *args);
  int n_args;
  const char *arg_types;
  const char *str;
  int allowed_states;
} commands[] = 
{
  /* Login command */
  MKCOMMAND(2, "ss", auth,        CL_CONNECTED),

  /* Readiness state. Both are valid in lobby, 'ready' is available in-game */
  MKCOMMAND(0, "",   ready,       CL_IN_LOBBY | CL_IN_GAME),
  MKCOMMAND(0, "",   notready,    CL_IN_LOBBY_ACK),

  /* General information commands. Can be used at any moment of time */
  MKCOMMAND(0, "",   quit,        CL_AUTHENTICATED),
  MKCOMMAND(0, "",   lslobby,     CL_AUTHENTICATED),
  MKCOMMAND(0, "",   lsgame,      CL_AUTHENTICATED),

  /* Game flow control */
  MKCOMMAND(0, "",   start,       CL_SUPERVISOR),
  MKCOMMAND(0, "",   step,        CL_SUPERVISOR),
  MKCOMMAND(0, "",   run,         CL_SUPERVISOR),
  MKCOMMAND(0, "",   pause,       CL_SUPERVISOR),
  MKCOMMAND(0, "",   abort,       CL_SUPERVISOR),

  /* In-game commands */
  MKCOMMAND(2, "uu", buy,         CL_IN_GAME),
  MKCOMMAND(2, "uu", sell,        CL_IN_GAME),
  MKCOMMAND(1, "u",  build,       CL_IN_GAME),
  MKCOMMAND(1, "u",  produce,     CL_IN_GAME)
};
const unsigned int n_commands = sizeof(commands)/sizeof(struct CommandDescription);
#undef MKCOMMAND

static int read_uint(const char *str, List **list)
{
  unsigned int t;
  if (sscanf(str, "%u", &t) != 1)
    return 0;
  else
    *list = list_push(*list, unsigned int, t);
  return 1;
}

static int check_args(List **pres, List *args, const struct CommandDescription *command)
{
  int i;
  List *res = NULL;
  if (list_size(args) != command->n_args)
    return 0;

  /* Convert arg types */
  i = 0;
  FOREACH(char *, arg, args)
  { 
    switch (command->arg_types[i])
    {
      case 'u':
        if (!read_uint(arg, &res))
        {
          list_delete(res);
          return 0;
        }
        break;
      case 's':
        res = list_push(res, char *, arg);
        break;
      default:
        fatal("Invalid argument type string");
    }
    i++;
  } FOREACH_END;
  *pres = list_reverse(res);
  return 1;
}


void command_exec(ServerData *d, ClientData *client, 
    const char *cmd_str, List *cmd_args)
{
  const char *msg = do_command_exec(d, client, cmd_str, cmd_args);
  if (msg == NULL)
    server_send_reply(d, client->fd, "ack");
  else
    server_send_reply(d, client->fd, "fail \"%s\"", msg);
}

static const char *do_command_exec(ServerData *d, ClientData *client, 
    const char *cmd_str, List *cmd_args)
{
  unsigned int i;
  for (i=0; i<n_commands; i++)
    if (strcmp(cmd_str, commands[i].str) == 0)
    {
      /* Command matched */
      if (commands[i].allowed_states & client->state)
      {
        /* Command allowed */
        List *processed_args;
        if (check_args(&processed_args, cmd_args, &commands[i]))
        {
          /* Command's arguments validated and parsed */
          const char *msg = commands[i].handler(d, client, processed_args);
          list_delete(processed_args);
          return msg;
        }
        else
          return "Bad command arguments";
      }
      else
        return "Command not available right now";
    }

  return "Bad command";
}


/* 
 * Commands' implementation
 */

static const char *cmd_auth(ServerData *d, ClientData *client, List *args)
{
  char *type = list_head(args, char *);
  char *name = list_head(args->next, char *);

  if (strlen(name)>MAXNAMELENGTH)
  {
    server_send_message(d, client->fd,
        "The length of your name must not exceed %d characters",
        MAXNAMELENGTH); 
    return "Name too long";
  } 
  else {
    FOREACH(ClientData *, client, d->clients)
    {
      if (client->name != NULL && strcmp(client->name, name) == 0)
        return "Name already used";
    } FOREACH_END;
    if (strcmp(type, "player") == 0)
    {
      server_set_client_state(d, client, CL_IN_LOBBY);
      client->name = strdup(name);
      server_send_message(d, client->fd,
          "Welcome to the lobby, %.100s",
          client->name); 
      message("%s has entered the lobby.", client->name);
    }
    else if (strcmp(type, "super") == 0)
    {
      server_set_client_state(d, client, CL_SUPERVISOR);
      client->name = strdup(name);
      server_send_message(d, client->fd,
          "Hi, %.100s, take your seat. I'm at your service.",
          client->name); 
      message("%s has connected as a supervisor", client->name);
    }
    else
      return "Bad client type";
  }
  return NULL;
}


static const char *cmd_quit(ServerData *d, ClientData *client, List *args)
{
  server_send_message(d, client->fd, "See ya!"); 
  socketloop_drop_client(d->loop, client->fd);
  /* Client will get disconnected somewhat later (asynchronously) */
  return NULL;
}


static const char *cmd_ready(ServerData *d, ClientData *client, List *args)
{
  if (client->state == CL_IN_LOBBY)
  {
    server_set_client_state(d, client, CL_IN_LOBBY_ACK);
    server_send_message(d, client->fd, 
        "You are ready to play. "
        "Please wait for the game to start"); 
    message("%s is ready to play.", client->name);
  }
  else if (client->state == CL_IN_GAME)
  {
    server_set_client_state(d, client, CL_IN_GAME_WAIT);
    server_send_message(d, client->fd,
        "Your turn is over. "
        "Please wait for other players to get ready.");
    message("%s has finished his turn.", client->name);
    d->n_waitfor--;
    if (d->n_waitfor == 0)
      fsm_switch_state(d->fsm, ST_BEFORE_ROUND);
  }
  return NULL;
}


static const char *cmd_notready(ServerData *d, ClientData *client, List *args)
{
  server_set_client_state(d, client, CL_IN_LOBBY);
  server_send_message(d, client->fd, "You are not ready to play."); 
  message("%s is not ready to play.", client->name);
  return NULL;
}


static const char *cmd_lslobby(ServerData *d, ClientData *client, List *args)
{
  FOREACH(ClientData *, item, d->clients)
  {
    if (item->state & (CL_IN_LOBBY | CL_IN_LOBBY_ACK))
      server_send_reply(d, client->fd, 
          "item \"%s\" %c",
          item->name,
          item->state==CL_IN_LOBBY?'-':'+');
  } FOREACH_END;
  return NULL;
}


static const char *cmd_lsgame(ServerData *d, ClientData *client, List *args)
{
  FOREACH(ClientData *, item, d->clients)
  {
    if (item->state & (CL_IN_GAME | CL_IN_GAME_WAIT))
      server_send_reply(d, client->fd, 
          "item \"%s\" %c %d %d %d %d",
          item->name,
          item->state==CL_IN_GAME?'-':'+',
          item->gcs.money, item->gcs.factories,
          item->gcs.raw, item->gcs.product);
  } FOREACH_END;
  return NULL;
}


static const char *cmd_start(ServerData *d, ClientData *client, List *args)
{
  count_t n_ready;
  if (d->fsm->state != ST_LOBBY)
    return "Game already started";

  n_ready = 0;
  FOREACH(ClientData *, client, d->clients)
  {
    if (client->state == CL_IN_LOBBY_ACK)
      n_ready++;
  } FOREACH_END; 
  if (n_ready < 2)
    return "Too few players ready. Need at least 2.";

  fsm_switch_state(d->fsm, ST_BEFORE_ROUND);
  return NULL;
}


static const char *cmd_step(ServerData *d, ClientData *client, List *args)
{
  if (d->fsm->state != ST_BEFORE_ROUND)
    return "Game not started or round already running";

  fsm_switch_state(d->fsm, ST_ROUND);
  return NULL;
}


static const char *cmd_run(ServerData *d, ClientData *client, List *args)
{
  if (d->fsm->state != ST_BEFORE_ROUND)
    return "Game not started or round already running";

  d->continuous_game = 1;
  fsm_switch_state(d->fsm, ST_ROUND);
  return NULL;
}


static const char *cmd_pause(ServerData *d, ClientData *client, List *args)
{
  if (d->fsm->state != ST_ROUND)
    return "Round not running";

  d->continuous_game = 0;
  return NULL;
}


static const char *cmd_abort(ServerData *d, ClientData *client, List *args)
{
  if (d->fsm->state != ST_ROUND && d->fsm->state != ST_BEFORE_ROUND)
    return "Not in game";

  d->abort_game = 1;
  fsm_switch_state(d->fsm, ST_LOBBY);
  return NULL;
}

static const char *cmd_buy(ServerData *d, ClientData *client, List *args)
{
  unsigned int count = list_head(args, unsigned int);
  unsigned int price = list_head(args->next, unsigned int);
  return game_request_buy(d, client, count, price);
}


static const char *cmd_sell(ServerData *d, ClientData *client, List *args)
{
  unsigned int count = list_head(args, unsigned int);
  unsigned int price = list_head(args->next, unsigned int);
  return game_request_sell(d, client, count, price);
}


static const char *cmd_produce(ServerData *d, ClientData *client, List *args)
{
  unsigned int count = list_head(args, unsigned int);
  return game_request_produce(d, client, count);
}


static const char *cmd_build(ServerData *d, ClientData *client, List *args)
{
  unsigned int count = list_head(args, unsigned int);
  return game_request_build(d, client, count);
}


