#include "StateCalibrateCenter.h"

#include "StateRun.h"

#include "AudioManager.h"

#include "TXMain.h"
#include "TXInput.h"

StateCalibrateCenter StateCalibrateCenter::instance;

//======================================================
//======================================================
void StateCalibrateCenter::onEnter(StateBase *prevState)
{
  this->stateTime = millis();

  TXInput::instance.calibrateAxisInitADC2();
}

//======================================================
//======================================================
void StateCalibrateCenter::onRun(uint32_t t)
{
  TXMain::instance.setLEDS4( 4 + 2 ); 

  TXInput::instance.loop(t);
  TXInput::instance.calibrateAxisAdjustMidMinMaxADC();

  if ( t - stateTime > 1000)
  {
    if ( TXInput::instance.isAxisCenterCalibrationSuccessfull() ) 
    {
      TXInput::instance.saveAxisCalibrationData();

      AudioManager::instance.play( "/calibration_finished.mp3", AUDIO_GROUP_NONE );
      AudioManager::instance.waitFinish();

      StateBase::Goto( &StateRun::instance );
    }
    else
    {
      StateBase::Goto( &StateCalibrateCenter::instance );
    }
  }
}
