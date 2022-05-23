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

  TXInput::instance.calibrateAxisInitADC2();

  if ( this->soundSkipCount == 0 ) 
  {
    AudioManager::instance.play( "/calibrate_center.mp3", AUDIO_GROUP_NONE );
  }
  this->soundSkipCount++;
  if ( this->soundSkipCount > 10 )
  {
    this->soundSkipCount = 0;
  }
}

//======================================================
//======================================================
void StateCalibrateWaitCenter::onRun(uint32_t t)
{
  if ( (t % 1000) < 500 )
  {
    TXMain::instance.setLEDS4( 4 + 2 ); 
  }
  else
  {
    TXMain::instance.setLEDS4( 0 ); 
  }

  TXInput::instance.loop(t);
  TXInput::instance.calibrateAxisAdjustMidMinMaxADC();

  if ( t - stateTime > 1000)
  {
    if ( TXInput::instance.isAxisCenterCalibrationSuccessfull() ) 
    {
      StateBase::Goto( &StateCalibrateCenter::instance );
    }
    else
    {
      StateBase::Goto( &StateCalibrateWaitCenter::instance );
    }
  }

}
