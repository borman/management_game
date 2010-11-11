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

#ifndef FSM_H
#define FSM_H

#include "list.h"

enum FSMEventType
{
  EV_CONNECT,
  EV_COMMAND,
  EV_DISCONNECT
};

typedef struct FSMEvent 
{
  enum FSMEventType type;
  int client;

  const char *command;
  List command_args;
} FSMEvent;

typedef struct FSM
{
  /* FSM description */
  const char *name;
  unsigned int n_states;
  const struct FSMState *states;
  
  /* User data */
  void *data;

  /* Current state */
  int state;
  int next_state;
  int loop_finished:1;
} FSM;

struct FSMState
{
  const char *name;
  void (*on_enter)(FSM *);
  void (*on_event)(FSM *, FSMEvent *);
  void (*on_exit)(FSM *);
};

void fsm_init(FSM *fsm, int init_state);
void fsm_event(FSM *fsm, FSMEvent *event);

void fsm_finish_loop(FSM *fsm);
void fsm_set_next_state(FSM *fsm, int state);

#endif /* FSM_H */

