#include "StateCalibrateWaitCenter.h"

#include "StateCalibrateCenter.h"

#include "AudioManager.h"

#include "TXMain.h"
#include "TXInput.h"

StateCalibrateWaitCenter StateCalibrateWaitCenter::instance;

//======================================================
//======================================================
void StateCalibrateWaitCenter::onEnter(StateBase *prevState)
{
  this->stateTime = millis();
  if ( prevState != &StateCalibrateWaitCenter::instance )
  {
    this->waitUnpress = true;
  }

  if ( this->count != 1 ) 
  {
    AudioManager::instance.play( (this->count & 1 ) == 0 ?  "/calibrate_center.mp3" : "/lb_start_calibration.mp3", AUDIO_GROUP_NONE );
  }
  if ( this->count == 0 ) AudioManager::instance.play( "/lb_start_calibration.mp3", AUDIO_GROUP_NONE );
  this->count++;
}

//======================================================
//======================================================
void StateCalibrateWaitCenter::onRun(uint32_t t)
{
  TXMain::instance.setLEDS4( 4 + 2 ); 

  TXInput::instance.loop(t);

  if ( this->waitUnpress )
  {
    if ( TXInput::instance.isButtonPressed( LEFT_BUMPER_ID) || TXInput::instance.isButtonPressed( LEFT_TRIGGER_ID) )
    {
      this->stateTime = millis();
    }
    else
    {
      this->waitUnpress = false;
    }
  }
  else
  {
    if ( t - stateTime > 7000 )
    {
      StateBase::Goto( &StateCalibrateWaitCenter::instance );
    }

    if ( TXInput::instance.isButtonPressed( LEFT_BUMPER_ID) || TXInput::instance.isButtonPressed( LEFT_TRIGGER_ID) )
    {
      StateBase::Goto( &StateCalibrateCenter::instance );
    }
  }
}
