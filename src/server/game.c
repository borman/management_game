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
#include <math.h>
#include "core/log.h"
#include "server/game.h"

#define INIT_MARKET_STATE 2
#define N_MARKET_STATES 5

const double market_state_prob[N_MARKET_STATES][N_MARKET_STATES] =
{
  {1.0/3.0,  1.0/3.0,  1.0/6.0, 1.0/12.0, 1.0/12.0},
  {1.0/4.0,  1.0/3.0,  1.0/4.0, 1.0/12.0, 1.0/12.0},
  {1.0/12.0, 1.0/4.0,  1.0/3.0, 1.0/4.0,  1.0/12.0},
  {1.0/12.0, 1.0/12.0, 1.0/4.0, 1.0/3.0,  1.0/4.0 },
  {1.0/12.0, 1.0/12.0, 1.0/6.0, 1.0/3.0,  1.0/3.0 }
};

const struct MarketState market_states[N_MARKET_STATES] =
{
/*  ==== RAW =    = PRODUCT = */
/*    N  PRICE      N   PRICE */
  {{1.0, 800.0}, {3.0, 6500.0}},
  {{1.5, 650.0}, {2.5, 6000.0}},
  {{2.0, 500.0}, {2.0, 5500.0}},
  {{2.5, 400.0}, {1.5, 5000.0}},
  {{3.0, 300.0}, {1.0, 4500.0}}
};


static void setup_state(ServerData *d);
static void send_ack(ServerData *d, int fd);


void game_start(ServerData *d)
{
  server_send_broadcast(d, CL_IN_GAME_WAIT, 
      "game start");
  d->market_state_number = INIT_MARKET_STATE;
  setup_state(d);
}


void game_finish_round(ServerData *d)
{
  server_send_broadcast(d, CL_IN_GAME_WAIT,
      "round end %u", 
      d->round_counter);
  server_send_broadcast(d, CL_IN_GAME_WAIT,
    "message Server \"The sales happen now.\"");
}

void game_start_round(ServerData *d)
{
  server_send_broadcast(d, CL_IN_GAME,
      "round start %u", 
      d->round_counter);
  server_send_broadcast(d, CL_IN_GAME,
      "market %u %lf %u %lf",
      d->market_state.raw.count, d->market_state.raw.price,
      d->market_state.product.count, d->market_state.product.price);
  FOREACH(ClientData *, client, d->clients)
  {
    static const GameClientRequest empty_req = {0,0,0,0,0,0};
    memcpy(&client->req, &empty_req, sizeof(GameClientRequest));
  } FOREACH_END;
}

void game_request_buy(ServerData *d, ClientData *client, 
    unsigned int count, double price)
{
  if (count > d->market_state.raw.count)
    server_send_message(d, client->fd,
        "error \"The Bank will not sell that many raw pieces.\"");
  else if (price < d->market_state.raw.price)
    server_send_message(d, client->fd,
        "error \"The price is too low.\"");
  else
  {
    trace("%s requested to buy %u for %lf", 
        client->name, count, price);
    client->req.raw_to_buy = count;
    send_ack(d, client->fd);
  }
}

void game_request_sell(ServerData *d, ClientData *client, 
    unsigned int count, double price)
{
  if (count > client->gcs.product)
    server_send_message(d, client->fd,
        "error \"You don't have enough pieces of product.\"");
  else if (count > d->market_state.product.count)
    server_send_message(d, client->fd,
        "error \"The Bank will not buy that many pieces of product.\"");
  else if (price > d->market_state.product.price)
    server_send_message(d, client->fd,
        "error \"The price is too high.\"");
  else
  {
    trace("%s requested to sell %u for %lf", 
        client->name, count, price);
    client->req.product_to_sell = count;
    send_ack(d, client->fd);
  }
}

void game_request_produce(ServerData *d, ClientData *client,
    unsigned int count)
{
  if (count > client->gcs.raw)
    server_send_message(d, client->fd,
        "error \"You don't have enough raw material "
        "to produce that many pieces of product.\"");
  else
  {
    client->req.items_to_produce = count;
    send_ack(d, client->fd);
  }
}

void game_request_build(ServerData *d, ClientData *client,
    unsigned int count)
{
}


static void setup_state(ServerData *d)
{
  memcpy(&d->market_state, &market_states[d->market_state_number], sizeof(MarketState));
  d->market_state.raw.count = round(d->n_players * d->market_state.raw.mult);
  d->market_state.product.count = round(d->n_players * d->market_state.product.mult);
}

static void send_ack(ServerData *d, int fd)
{
  server_send_message(d, fd,
      "ack");
}
