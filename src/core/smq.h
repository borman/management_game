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


#ifndef SMQ_H
#define SMQ_H

/**
 * SocketMessageQueue
 *
 * A socket-oriented message queue.
 * Sends string messages terminated with newline charachters
 */

typedef struct SocketMessageQueue SocketMessageQueue;

SocketMessageQueue *smq_new();
void smq_delete(SocketMessageQueue *smq);

void smq_enqueue(SocketMessageQueue *smq, const char *message);
void smq_try_send(SocketMessageQueue *smq, int fd);

int smq_is_empty(SocketMessageQueue *smq);

#endif /* SMQ_H */
