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


#ifndef SERVER_FSM_H
#define SERVER_FSM_H

#include "fsm.h"
#include "socket_loop.h"

enum ClientState
{
  CL_CONNECTED    = 0x01,
  CL_SUPERVISOR   = 0x02,
  CL_IN_LOBBY     = 0x04,
  CL_IN_LOBBY_ACK = 0x08,
  CL_IN_GAME      = 0x10,
  CL_DEAD         = 0x20
};

FSM *server_fsm_new(SocketLoop *loop);
void server_fsm_delete(FSM *fsm);

#endif /* SERVER_FSM_H */

