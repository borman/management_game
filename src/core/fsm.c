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


#include "core/fsm.h"
#include "core/log.h"


void fsm_init(FSM *fsm, int init_state)
{
  fsm->state = init_state;
  fsm->next_state = init_state;
  fsm->loop_finished = 0;
  
  fsm->states[fsm->state].on_enter(fsm);
}


void fsm_event(FSM *fsm, FSMEvent *event)
{
  fsm->states[fsm->state].on_event(fsm, event);
  while (fsm->loop_finished)
  {
    fsm->states[fsm->state].on_exit(fsm);
    fsm->state = fsm->next_state;
    fsm->loop_finished = 0;
    fsm->states[fsm->state].on_enter(fsm);
  }
}


void fsm_switch_state(FSM *fsm, int state)
{
  fsm->next_state = state;
  fsm->loop_finished = 1;
}



