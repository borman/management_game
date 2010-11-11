#ifndef SERVER_FSM_H
#define SERVER_FSM_H

#include "fsm.h"
#include "socket_loop.h"

FSM *server_fsm_new(SocketLoop *loop);
void server_fsm_delete(FSM *fsm);

#endif /* SERVER_FSM_H */

