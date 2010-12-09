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



