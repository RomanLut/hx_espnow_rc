#include "StateBase.h"


StateBase* StateBase::currentState = NULL;

//======================================================
//======================================================
void StateBase::Goto(StateBase* s)
{
  StateBase::currentState = s;
  StateBase::currentState->onEnter();
}
