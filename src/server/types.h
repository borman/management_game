#ifndef TYPES_H
#define TYPES_H

#include "core/list.h"
#include "core/fsm.h"
#include "core/socket_loop.h"

/* Semantic int types */
typedef unsigned int count_t;
typedef unsigned int price_t;
typedef int money_t;

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

enum DealType
{
  INCOME,
  EXPENDITURE
};

typedef struct FactoryRequest
{
  count_t count;
  int deadline;
} FactoryRequest;

typedef struct MarketState
{
  struct 
  { 
    double mult;
    price_t price;
    count_t count;
  } raw, product; 
} MarketState;

typedef struct GameClientState
{
  money_t money;
  count_t factories;
  count_t raw;
  count_t product;
  /* List of factories being built */
  List factories_incomplete; /* List<FactoryRequest *> */
} GameClientState;

typedef struct GameClientRequest
{
  count_t raw_to_buy;
  price_t raw_price;
  count_t product_to_sell;
  price_t product_price;
  count_t items_to_produce;
  count_t factories_to_build;
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
  count_t n_players;
  /* Cound of players whose turns are not finished */
  count_t n_waitfor;

  /* Whether to start the next round automatically without waiting 
   * for a supervisor's permission */
  unsigned int continuous_game:1;
  /* Whether the game is ending because abort was requested */
  unsigned int abort_game:1;
} ServerData;

#endif /* TYPES_H */

