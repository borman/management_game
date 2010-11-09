#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "smq.h"
#include "list.h"
#include "debug.h"

typedef struct Message
{
  size_t size;
  char *data;
} Message;

struct SocketMessageQueue 
{
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
  } FOREACH_END
  list_delete(smq->queued);
  free(smq);
}


void smq_enqueue(SocketMessageQueue *smq, const char *message)
{
  smq->queued = list_push_back(smq->queued, Message *, message_new(message));
}


void smq_try_send(SocketMessageQueue *smq, int fd)
{
  trace("smq_try_send(%d)", fd);
}


int smq_is_empty(SocketMessageQueue *smq)
{
  return smq->current == NULL && smq->queued == NULL;
}



static Message *message_new(const char *str)
{
  Message *msg = (Message *) malloc(sizeof(Message));
  const size_t size = strlen(str) + 1;
  char *data = malloc(msg->size+1);
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


