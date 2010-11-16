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


#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "core/smq.h"
#include "core/list.h"
#include "core/log.h"

typedef struct Message
{
  size_t size;
  char *data;
} Message;

struct SocketMessageQueue 
{
  size_t current_pos;
  Message *current;
  List queued;
};



static Message *message_new(const char *str);
static void message_delete(Message *msg);



SocketMessageQueue *smq_new()
{
  SocketMessageQueue *smq = (SocketMessageQueue *) calloc(1, sizeof(SocketMessageQueue));
  return smq;
}


void smq_delete(SocketMessageQueue *smq)
{
  if (smq->current != NULL)
    message_delete(smq->current);
  FOREACH(Message *, msg, smq->queued)
  {
    message_delete(msg);
  } FOREACH_END;
  list_delete(smq->queued);
  free(smq);
}


void smq_enqueue(SocketMessageQueue *smq, const char *message)
{
  smq->queued = list_push_back(smq->queued, Message *, message_new(message));
}


void smq_try_send(SocketMessageQueue *smq, int fd)
{
  while (!smq_is_empty(smq))
  {
    while (smq->current != NULL && smq->current_pos < smq->current->size)
    {
      /* send */
      ssize_t delta = send(fd, 
          smq->current->data + smq->current_pos, 
          smq->current->size - smq->current_pos, 
          MSG_DONTWAIT);
      if (delta == -1)
      {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
          warning("Send failed: %s", strerror(errno));
        return;
      }
      else
        smq->current_pos += delta;
    }
    if (smq->current != NULL)
    {
      /* current sent -> dispose */
      message_delete(smq->current);
      smq->current = NULL;
    } 
    if (smq->queued != NULL)
    {
      /* dequeue */
      smq->current = list_head(smq->queued, Message *);
      smq->current_pos = 0;
      smq->queued = list_pop(smq->queued);
    }
  }
}


int smq_is_empty(SocketMessageQueue *smq)
{
  return smq->current == NULL && smq->queued == NULL;
}



static Message *message_new(const char *str)
{
  Message *msg = (Message *) malloc(sizeof(Message));
  const size_t size = strlen(str) + 1;
  char *data = malloc(size+1);
  strcpy(data, str);
  data[size-1] = '\n';
  data[size] = '\0';
  msg->size = size;
  msg->data = data;
  return msg;
}


static void message_delete(Message *msg)
{
  free(msg->data);
  free(msg);
}


