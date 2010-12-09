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
#include <stdlib.h>
#include <assert.h>
#include "core/log.h"
#include "server/game.h"

#define INIT_MARKET_STATE 2
#define N_MARKET_STATES 5

#define min(a, b) (((a)<(b))?(a):(b))
#define client_in_game(client) ((client)->state & (CL_IN_GAME|CL_IN_GAME_WAIT))

static const GameClientState player_init_state = 
{
  10000, /* money */
  2, /* factories */
  4, /* raw */
  2, /* product */
  NULL
};

static const double market_state_prob[N_MARKET_STATES][N_MARKET_STATES] =
{
  {1.0/3.0,  1.0/3.0,  1.0/6.0, 1.0/12.0, 1.0/12.0},
  {1.0/4.0,  1.0/3.0,  1.0/4.0, 1.0/12.0, 1.0/12.0},
  {1.0/12.0, 1.0/4.0,  1.0/3.0, 1.0/4.0,  1.0/12.0},
  {1.0/12.0, 1.0/12.0, 1.0/4.0, 1.0/3.0,  1.0/4.0 },
  {1.0/12.0, 1.0/12.0, 1.0/6.0, 1.0/3.0,  1.0/3.0 }
};

static const struct MarketState market_states[N_MARKET_STATES] =
{
/*  ==== RAW = = PRODUCT = */
/*    N  PRICE    N  PRICE */
  {{1.0, 800}, {3.0, 6500}},
  {{1.5, 650}, {2.5, 6000}},
  {{2.0, 500}, {2.0, 5500}},
  {{2.5, 400}, {1.5, 5000}},
  {{3.0, 300}, {1.0, 4500}}
};

/* Financial operations */
static void do_finances(ServerData *d);
static void do_costs(ServerData *d);
static void do_kill_bankrupts(ServerData *d);
static void finance(ServerData *d, ClientData *client, enum DealType type,
   const char *purpose_major, const char *purpose_minor, 
   count_t count, price_t price);

/* Utilities to manage market states */
static void setup_state(ServerData *d);
static void update_market_state(ServerData *d);

/* Price sorting helpers */
static int compare_price(price_t a, price_t b);
static int compare_buy_raw(const void *pa, const void *pb);
static int compare_sell_product(const void *pa, const void *pb);

/* Select a random value in range 0..count-1 with given distribution */
static int dice(const double *probs, unsigned int count);

void game_start(ServerData *d)
{
  d->market_state_number = INIT_MARKET_STATE;
  setup_state(d);
  /* Init players' states */
  FOREACH(ClientData *, client, d->clients)
  {
    if (client_in_game(client))
      memcpy(&client->gcs, &player_init_state, sizeof(GameClientState));
  } FOREACH_END;
}


void game_finish_round(ServerData *d)
{
  server_send_broadcast(d, CL_IN_GAME_WAIT,
      "round end %u", 
      d->round_counter);
  update_market_state(d);
  do_finances(d);
  do_kill_bankrupts(d);
  game_check_players(d);
}

void game_start_round(ServerData *d)
{
  server_send_broadcast(d, CL_IN_GAME,
      "round start %u", 
      d->round_counter);
  server_send_broadcast(d, CL_IN_GAME,
      "market %u %u %u %u",
      d->market_state.raw.count, d->market_state.raw.price,
      d->market_state.product.count, d->market_state.product.price);
  /* Init request data */
  FOREACH(ClientData *, client, d->clients)
  {
    static const GameClientRequest empty_req = {0,0,0,0,0,0};
    if (client_in_game(client))
      memcpy(&client->req, &empty_req, sizeof(GameClientRequest));
  } FOREACH_END;
}

void game_remove_player(ServerData *d, ClientData *client)
{
  server_set_client_state(d, client, CL_IN_LOBBY);
  d->n_players--;
  if (d->fsm->state == ST_ROUND && client->state == CL_IN_GAME)
    d->n_waitfor--;
}

void game_check_players(ServerData *d)
{
  if (d->n_players == 0)
  {
    message("The game is over and nobody is left alive.");
    fsm_switch_state(d->fsm, ST_LOBBY);
  }
  else if (d->n_players == 1)
  {
    ClientData *winner = NULL;
    FOREACH(ClientData *, client, d->clients)
    {
      if (client_in_game(client))
        winner = client;
    } FOREACH_END;
    assert(winner != NULL);
    message("The game is over. And the winner is... %s!", winner->name);
    game_remove_player(d, winner);
    fsm_switch_state(d->fsm, ST_LOBBY);
  }
}

const char *game_request_buy(ServerData *d, ClientData *client, 
    count_t count, price_t price)
{
  if (count > d->market_state.raw.count)
    return "The Bank will not sell that many raw pieces.";
  else if (price < d->market_state.raw.price)
    return "The price is too low.";
  else
  {
    trace("%s requested to buy %u for %u", 
        client->name, count, price);
    client->req.raw_to_buy = count;
    client->req.raw_price = price;
  }
  return NULL;
}

const char *game_request_sell(ServerData *d, ClientData *client, 
    count_t count, price_t price)
{
  if (count > client->gcs.product)
    return "You don't have enough pieces of product.";
  else if (count > d->market_state.product.count)
    return "The Bank will not buy that many pieces of product.";
  else if (price > d->market_state.product.price)
    return "The price is too high.";
  else
  {
    trace("%s requested to sell %u for %u", 
        client->name, count, price);
    client->req.product_to_sell = count;
    client->req.product_price = price;
  }
  return NULL;
}

const char *game_request_produce(ServerData *d, ClientData *client, count_t count)
{
  if (count > client->gcs.raw)
    return "You don't have enough raw material "
      "to produce that many pieces of product.";
  else if (count > client->gcs.factories)
    return "You don't have enough factories "
      "to produce that many pieces of product.";
  else
  {
    trace("%s requested to produce %u items", 
        client->name, count);
    client->req.items_to_produce = count;
  }
  return NULL;
}

const char *game_request_build(ServerData *d, ClientData *client, count_t count)
{
  client->req.factories_to_build = count;
  return NULL;
}


static int compare_price(price_t a, price_t b)
{
  if (a<b)
    return -1;
  else if (a>b)
    return 1;
  else
    return 0;
}

static int compare_buy_raw(const void *pa, const void *pb)
{
  price_t a = (*((ClientData * const*) pa))->req.raw_price;
  price_t b = (*((ClientData * const*) pb))->req.raw_price;
  /* results must be descending */
  return -compare_price(a, b);
}

static int compare_sell_product(const void *pa, const void *pb)
{
  price_t a = (*((ClientData * const*) pa))->req.product_price;
  price_t b = (*((ClientData * const*) pb))->req.product_price;
  /* results must be ascending */
  return compare_price(a, b);
}

static void do_finances(ServerData *d)
{
  count_t n_players = d->n_players;
  ClientData **offers = (ClientData **) malloc(sizeof(ClientData *) * n_players);
  unsigned int i = 0;
  count_t n_goods;

  /* Prepare player array for sorting */
  FOREACH(ClientData *, client, d->clients)
  {
    if (client_in_game(client))
      offers[i++] = client;
  } FOREACH_END;
  
  /* Sell raw to players */
  qsort(offers, n_players, sizeof(ClientData *), compare_buy_raw);
  n_goods = d->market_state.raw.count;
  for (i=0; i<n_players && n_goods>0; i++)
  {
    ClientData *client = offers[i];
    count_t to_sell = min(client->req.raw_to_buy, n_goods);
    client->gcs.raw += to_sell;
    n_goods -= to_sell;
    trace ("Sell %u raw to %s", to_sell, client->name);
    finance(d, client, EXPENDITURE, "auction", "raw", to_sell, client->req.raw_price);
  }

  /* Buy product from players */
  qsort(offers, n_players, sizeof(ClientData *), compare_sell_product);
  n_goods = d->market_state.product.count;
  for (i=0; i<n_players && n_goods>0; i++)
  {
    ClientData *client = offers[i];
    count_t to_buy = min(client->req.product_to_sell, n_goods);
    client->gcs.product -= to_buy;
    n_goods -= to_buy;
    trace ("Buy %u product from %s", to_buy, client->name);
    finance(d, client, INCOME, "auction", "product", to_buy, client->req.product_price);
  }

  do_costs(d);
  free(offers);
  /* kill bankrupts */
}

/* Charge for pending stuff */
static void do_costs(ServerData *d)
{
  FOREACH(ClientData *, client, d->clients)
  {
    if (client_in_game(client))
    {
      List *factories_incomplete;

      /* Posession cost */
      finance(d, client, EXPENDITURE, "cost", "factory", 
          client->gcs.factories, 1000);
      finance(d, client, EXPENDITURE, "cost", "raw", 
          client->gcs.raw, 300);
      finance(d, client, EXPENDITURE, "cost", "product", 
          client->gcs.product, 500);

      /* Production */
      finance(d, client, EXPENDITURE, "cost", "production", 
          client->req.items_to_produce, 2000);
      client->gcs.product += client->req.items_to_produce;

      /* Factory construction */
      factories_incomplete = client->gcs.factories_incomplete;
      if (factories_incomplete != NULL)
      {
        FactoryRequest *req = list_head(factories_incomplete, FactoryRequest *); 
        if (req->deadline == d->round_counter + 1)
        {
          finance(d, client, EXPENDITURE, "cost", "construction_end", 
            req->count, 5000/2);
          client->gcs.factories += req->count;
          factories_incomplete = list_pop(factories_incomplete);
          free(req);
        }
      }
      finance(d, client, EXPENDITURE, "cost", "construction_begin", 
          client->req.factories_to_build, 5000/2);
      if (client->req.factories_to_build > 0)
      {
        FactoryRequest *req = (FactoryRequest *) malloc(sizeof(FactoryRequest));
        req->count = client->req.factories_to_build;
        req->deadline = d->round_counter + 5;
        factories_incomplete = 
          list_push_back(factories_incomplete, FactoryRequest *, req);
      }
      client->gcs.factories_incomplete = factories_incomplete;
    }
  } FOREACH_END;
}

static void do_kill_bankrupts(ServerData *d)
{
  FOREACH(ClientData *, client, d->clients)
  {
    if (client_in_game(client) && client->gcs.money<0)
    {
      message("%s is a bankrupt. He has to leave the game.", client->name);
      game_remove_player(d, client);
    }
  } FOREACH_END;
}

static void setup_state(ServerData *d)
{
  memcpy(&d->market_state, &market_states[d->market_state_number], sizeof(MarketState));
  d->market_state.raw.count = round(d->n_players * d->market_state.raw.mult);
  d->market_state.product.count = round(d->n_players * d->market_state.product.mult);
}

static void update_market_state(ServerData *d)
{
  int old_state = d->market_state_number;
  int new_state = dice(market_state_prob[old_state], N_MARKET_STATES);
  if (old_state != new_state)
  {
    trace("Market state change: %d -> %d", old_state, new_state);
    d->market_state_number = new_state;
    setup_state(d);
  }
}

static void finance(ServerData *d, ClientData *client, enum DealType type,
   const char *purpose_major, const char *purpose_minor, 
   count_t count, price_t price)
{
  money_t delta = (type==INCOME? 1 : -1) * count * price;
  if (delta == 0)
    return;
  
  server_send_broadcast(d, CL_IN_GAME_WAIT, 
      "finance \"%s\" %s %s %u %u %+d",
      client->name,
      purpose_major, purpose_minor,
      count, price, delta);
  client->gcs.money += delta;
}

static int dice(const double *probs, unsigned int count)
{
  double acc = 0;
  int i;
  int randval = rand();
  for (i=0; i<count; i++)
  {
    acc += probs[i];
    if (randval <= (acc*RAND_MAX))
      return i;
  }
  return count-1;
}


