#include "core/fsm.h"
#include "core/log.h"


void fsm_init(FSM *fsm, int init_state)
{
  trace("FSM %s init(%s)", fsm->name, fsm->states[init_state].name);
  fsm->state = init_state;
  fsm->next_state = init_state;
  fsm->loop_finished = 0;
  
  fsm->states[fsm->state].on_enter(fsm);
}


void fsm_event(FSM *fsm, FSMEvent *event)
{
  trace("FSM %s(%s) event %d from %d", 
      fsm->name, fsm->states[fsm->state].name,
      event->type, event->fd);

  fsm->states[fsm->state].on_event(fsm, event);
  if (fsm->loop_finished)
  {
    trace("FSM %s(%s) loop finished -> switch state", 
        fsm->name, fsm->states[fsm->state].name);

    fsm->states[fsm->state].on_exit(fsm);
    
    trace("FSM %s: %s -> %s", fsm->name, 
        fsm->states[fsm->state].name, 
        fsm->states[fsm->next_state].name);
    fsm->state = fsm->next_state;

    fsm->states[fsm->state].on_enter(fsm);

    fsm->loop_finished = 0;
  }
}


void fsm_finish_loop(FSM *fsm)
{
  trace("FSM %s(%s) req finish loop", 
      fsm->name, fsm->states[fsm->state].name);
  fsm->loop_finished = 1;
}


void fsm_set_next_state(FSM *fsm, int state)
{
  fsm->next_state = state;
}



