#include "events.h"
#include "debug.h"



void event_client_connected(int client)
{
  trace("EVENT: %d: connected", client);
}


void event_client_incoming_message(int client, const char *message)
{
  trace("EVENT: %d: incoming: %s", client, message);
}


void event_client_disconnected(int client)
{
  trace("EVENT: %d: disconnected", client);
}

