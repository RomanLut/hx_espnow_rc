#include "StateBase.h"
#include "TXMain.h"


StateBase* StateBase::currentState = NULL;

//======================================================
//======================================================
void StateBase::Goto(StateBase* s)
{
  StateBase::currentState = s;
  StateBase::currentState->onEnter();
}

//======================================================
//======================================================
void StateBase::SetLEDS4Profile(uint8_t profileIndex)
{
  switch ( profileIndex)
  {
    case 0:
      TXMain::instance.setLEDS4( 8 );
      break;

    case 1:
      TXMain::instance.setLEDS4( 4 );
      break;

    case 2:
      TXMain::instance.setLEDS4( 2 );
      break;

    case 3:
      TXMain::instance.setLEDS4( 1 );
      break;

    case 4:
      TXMain::instance.setLEDS4( 4 + 2 + 1 );
      break;

    case 5:
      TXMain::instance.setLEDS4( 8 + 2 + 1 );
      break;

    case 6:
      TXMain::instance.setLEDS4( 8 + 4 + 1 );
      break;

    case 7:
      TXMain::instance.setLEDS4( 8 + 4 + 2 );
      break;
  }
}

