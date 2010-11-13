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


#ifndef SOCKET_LOOP_H
#define SOCKET_LOOP_H

typedef struct SocketLoop SocketLoop;
typedef struct SocketLoopEventHandler
{
  void (*on_client_connect)(SocketLoop *loop, int client);
  void (*on_incoming_message)(SocketLoop *loop, int client, const char *message);
  void (*on_client_disconnect)(SocketLoop *loop, int client);
} SocketLoopEventHandler;

SocketLoop *socketloop_new(const SocketLoopEventHandler *handler);
void socketloop_delete(SocketLoop *loop);

void socketloop_listen(SocketLoop *loop, int fd);
void socketloop_close_listeners(SocketLoop *loop);

void socketloop_run(SocketLoop *loop);
void socketloop_stop(SocketLoop *loop);

void socketloop_send(SocketLoop *loop, int client, const char *command);
void socketloop_drop_client(SocketLoop *loop, int client);

void socketloop_set_data(SocketLoop *loop, void *data);
void *socketloop_get_data(SocketLoop *loop);
#endif /* SOCKET_LOOP_H */
