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
