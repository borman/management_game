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


#include "events.h"
#include "debug.h"
#include "socket_loop.h"
#include "list.h"
#include "lexer.h"
#include "fsm.h"


static void event_client_incoming_command(SocketLoop *loop, int client, const char *command, List args);


void event_client_connected(SocketLoop *loop, int client)
{
  FSMEvent event = {EV_CONNECT, client};
  fsm_event((FSM *) socketloop_get_data(loop), &event);
}


void event_client_incoming_message(SocketLoop *loop, int client, const char *message)
{
  TokenList *tl;
  tl = lexer_split(message);
  if (tl == NULL)
  {
    trace("Bad message from %d", client);
    socketloop_send(loop, client, "error \"Bad syntax\"");
    socketloop_drop_client(loop, client);
  }
  else if (tl->tokens != NULL)
  {
    const char *command = list_head(tl->tokens, char *);
    List args = tl->tokens->next;
    event_client_incoming_command(loop, client, command, args);
    lexer_delete(tl);
  }
}


void event_client_disconnected(SocketLoop *loop, int client)
{
  FSMEvent event = {EV_DISCONNECT, client};
  fsm_event((FSM *) socketloop_get_data(loop), &event);
}



static void event_client_incoming_command(SocketLoop *loop, int client, 
    const char *command, List args)
{
  FSMEvent event = {EV_COMMAND, client, command, args};
  fsm_event((FSM *) socketloop_get_data(loop), &event);
}

