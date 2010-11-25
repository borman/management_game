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


#ifndef SERVER_FSM_H
#define SERVER_FSM_H

#include "core/fsm.h"
#include "core/socket_loop.h"

enum ClientState
{
  CL_CONNECTED           = 0x01,
  CL_SUPERVISOR          = 0x02,
  CL_IN_LOBBY            = 0x04,
  CL_IN_LOBBY_ACK        = 0x08,
  CL_IN_GAME             = 0x10,
  CL_IN_GAME_WAIT        = 0x20,
  CL_DEAD                = 0x40
};

enum ServerState
{
  ST_LOBBY = 0,
  ST_BEFORE_ROUND,
  ST_ROUND
};

typedef struct FactoryRequest
{
  int count;
  int deadline;
} FactoryRequest;

typedef struct MarketState
{
  struct 
  { 
    double mult;
    double price;
    unsigned int count;
  } raw, product; 
} MarketState;

typedef struct GameClientState
{
  int money;
  int factories;
  int raw;
  int product;
  /* List of factories being built */
  List factories_incomplete; /* List<FactoryRequest *> */
} GameClientState;

typedef struct GameClientRequest
{
  unsigned int raw_to_buy;
  double raw_price;
  unsigned int product_to_sell;
  double product_price;
  int items_to_produce;
  int factories_to_build;
} GameClientRequest;

typedef struct ClientData
{
  int fd;
  enum ClientState state;
  char *name;
  GameClientState gcs;
  GameClientRequest req;
} ClientData;

typedef struct ServerData
{
  /* Link back to FSM */
  FSM *fsm;

  SocketLoop *loop;
  List clients; /* List<ClientData *> */

  /* Current round's number */
  unsigned int round_counter;
  /* Current market state */
  unsigned int market_state_number;
  MarketState market_state;

  /* Count of players in-game */
  unsigned int n_players;
  /* Cound of players whose turns are not finished */
  unsigned int n_waitfor;

  /* Whether to start the next round automatically without waiting 
   * for a supervisor's permission */
  unsigned int continuous_game:1;
  /* Whether the game is ending because abort was requested */
  unsigned int abort_game:1;
} ServerData;


FSM *server_fsm_new(SocketLoop *loop);
void server_fsm_delete(FSM *fsm);

void server_send_message(ServerData *d, int fd, const char *format, ...);
void server_send_broadcast(ServerData *d, int client_mask, const char *format, ...);

#endif /* SERVER_FSM_H */

