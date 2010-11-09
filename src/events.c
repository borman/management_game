#include "events.h"
#include "debug.h"
#include "socket_loop.h"
#include "list.h"
#include "lexer.h"


static void event_client_incoming_command(SocketLoop *loop, int client, const char *command, List args);


void event_client_connected(SocketLoop *loop, int client)
{
  trace("EVENT: %d: connected", client);
}


void event_client_incoming_message(SocketLoop *loop, int client, const char *message)
{
  TokenList *tl;
  trace("EVENT: %d: incoming: %s", client, message);
  tl = lexer_split(message);
  if (tl == NULL)
  {
    trace("Bad message");
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
  trace("EVENT: %d: disconnected", client);
}



static void event_client_incoming_command(SocketLoop *loop, int client, const char *command, List args)
{
  trace("EVENT: %d: %s", client, command);
  trace("Echoing back: %s", command);
  socketloop_send(loop, client, command);
}

