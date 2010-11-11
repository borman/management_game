#include <stdlib.h>
#include <string.h>

#include "server_fsm.h"

enum ServerState
{
  ST_LOBBY = 0,
  ST_ROUND
};

enum ClientState
{
  CL_CONNECTED,
  CL_SUPERVISOR,
  CL_CLIENT,
  CL_IN_LOBBY,
  CL_IN_LOBBY_ACK,
  CL_IN_GAME
};

typedef struct ClientData
{
  int fd;
  enum ClientState state;
} ClientData;

typedef struct ServerData
{
  SocketLoop *loop;
  List clients; /* List<ClientData *> */
} ServerData;



static ClientData *new_client(int fd);
static ClientData *find_client(ServerData *d, int fd);
static void delete_client(ClientData *client); 

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
  ServerData *d = (ServerData *) fsm->data;
}


static void lobby_on_event(FSM *fsm, FSMEvent *event)
{
  ServerData *d = (ServerData *) fsm->data;
  switch (event->type)
  {
    case EV_CONNECT:
      break;
    case EV_DISCONNECT:
      break;
    case EV_COMMAND:
      break;
  }
}


static void lobby_on_exit(FSM *fsm)
{
  ServerData *d = (ServerData *) fsm->data;
}


static void round_on_enter(FSM *fsm)
{
  ServerData *d = (ServerData *) fsm->data;
}


static void round_on_event(FSM *fsm, FSMEvent *event)
{
  ServerData *d = (ServerData *) fsm->data;
}


static void round_on_exit(FSM *fsm)
{
  ServerData *d = (ServerData *) fsm->data;
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
  } FOREACH_END
  fatal("server_fsm: Nonexistent client requested: %d", client);
  return NULL;
}


static void delete_client(ClientData *client)
{
  free(client);
}

