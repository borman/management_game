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


#ifndef GAME_H
#define GAME_H

#include "server/server_fsm.h"

void game_start(ServerData *d);

void game_start_round(ServerData *d);
void game_finish_round(ServerData *d);

const char *game_request_buy(ServerData *d, ClientData *client, 
    count_t count, price_t price);
const char *game_request_sell(ServerData *d, ClientData *client, 
    count_t count, price_t price);
const char *game_request_produce(ServerData *d, ClientData *client,
    count_t count);
const char *game_request_build(ServerData *d, ClientData *client,
    count_t count);

#endif /* GAME_H */

