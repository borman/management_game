#include <stdlib.h>
#include <string.h>

#include "server_fsm.h"

enum ServerState
{
  ST_LOBBY = 0,
  ST_ROUND
};

typedef struct ClientData
{
  int fd;
} ClientData;

typedef struct ServerData
{
  SocketLoop *loop;
  List clients; /* List<ClientData *> */
} ServerData;

static void lobby_on_enter(FSM *fsm);
static void lobby_on_event(FSM *fsm, FSMEvent *event);
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
}


static void lobby_on_event(FSM *fsm, FSMEvent *event)
{
}


static void lobby_on_exit(FSM *fsm)
{
}


static void round_on_enter(FSM *fsm)
{
}


static void round_on_event(FSM *fsm, FSMEvent *event)
{
}


static void round_on_exit(FSM *fsm)
{
}


