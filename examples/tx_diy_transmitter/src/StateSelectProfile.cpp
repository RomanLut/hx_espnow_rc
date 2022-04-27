#include "StateSelectProfile.h"

#include "StateRun.h"

#include "TXMain.h"
#include "TXInput.h"

StateSelectProfile StateSelectProfile::instance;

//======================================================
//======================================================
void StateSelectProfile::onEnter()
{
  this->stateTime = millis();
  this->waitUnpress = true;
  this->exit = false;
  this->profileId = 0;

  TXMain::instance.setLEDS4( 8 + 4 + 2 + 1 );
}

//======================================================
//======================================================
void StateSelectProfile::SetLEDS4Profile()
{
  switch ( this->profileId)
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

//======================================================
//======================================================
void StateSelectProfile::onRun(uint32_t t)
{
  TXInput::instance.loop(t);

  if ( this->waitUnpress )
  {
    if ( 
      TXInput::instance.isButtonUnPressed(LEFT_BUMPER_ID) && 
      TXInput::instance.isStickMiddle(RIGHT_STICK_X_ID)
      )
    {
      this->waitUnpress = false;
    }
  }
  else
  {
    if ( this->exit ) 
    {
      StateBase::Goto(&StateRun::instance);
    }
    if ( t % 256 < 128 )
    {
      this->SetLEDS4Profile();
    }
    else
    {
      TXMain::instance.setLEDS4( 0 );
    }

    if ( TXInput::instance.isStickMin(RIGHT_STICK_X_ID) )
    {
      this->waitUnpress = true;
      if ( profileId == 0)
      {
        profileId = 7;
      }
      else
      {
        profileId--;
      }
      this->SetLEDS4Profile();
    }

    if ( TXInput::instance.isStickMax(RIGHT_STICK_X_ID) )
    {
      this->waitUnpress = true;
      if ( profileId == 7)
      {
        profileId = 0;
      }
      else
      {
        profileId++;
      }
      this->SetLEDS4Profile();
    }

    if ( TXInput::instance.isButtonPressed(LEFT_BUMPER_ID) )
    {
      this->waitUnpress = true;
      this->exit = true;
    }
  }
}
