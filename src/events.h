#ifndef EVENTS_H
#define EVENTS_H

#include "socket_loop.h"

void event_client_connected(SocketLoop *loop, int client);
void event_client_incoming_message(SocketLoop *loop, int client, const char *message);
void event_client_disconnected(SocketLoop *loop, int client);

#endif /* EVENTS_H */
