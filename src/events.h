#ifndef EVENTS_H
#define EVENTS_H

void event_client_connected(int client);
void event_client_incoming_message(int client, const char *message);
void event_client_disconnected(int client);

#endif /* EVENTS_H */
