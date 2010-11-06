#include "events.h"
#include "debug.h"
#include "socket_loop.h"
#include "list.h"
#include "lexer.h"


static void event_client_incoming_command(int client, const char *command, List args);


void event_client_connected(int client)
{
  trace("EVENT: %d: connected", client);
}


void event_client_incoming_message(int client, const char *message)
{
  TokenList *tl;
  trace("EVENT: %d: incoming: %s", client, message);
  tl = lexer_split(message);
  if (tl == NULL)
  {
    trace("Bad message");
    socketloop_send(client, "error \"Bad syntax\"");
    socketloop_drop_client(client);
  }
  else if (tl->tokens != NULL)
  {
    const char *command = list_head(tl->tokens, char *);
    List args = tl->tokens->next;
    event_client_incoming_command(client, command, args);
    lexer_delete(tl);
  }
}


void event_client_disconnected(int client)
{
  trace("EVENT: %d: disconnected", client);
}



static void event_client_incoming_command(int client, const char *command, List args)
{
  trace("EVENT: %d: %s", client, command);
}

