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
  if ( prevState != &StateCalibrateCenter::instance )
  {
    this->waitUnpress = true;
  }

  AudioManager::instance.play( "/lb_finish.mp3", AUDIO_GROUP_NONE );
}

//======================================================
//======================================================
void StateCalibrateCenter::onRun(uint32_t t)
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

  if ( this->waitUnpress )
  {
    if ( TXInput::instance.isButtonPressed( LEFT_BUMPER_ID) || TXInput::instance.isButtonPressed( LEFT_TRIGGER_ID) )
    {
      this->stateTime = millis();
    }
    else
    {
      this->waitUnpress = false;
      TXInput::instance.calibrateAxisInitADC2();
    }
  }
  else
  {
    if ( t - stateTime > 7000)
    {
      TXInput::instance.dumpAxisCenterCalibration();
      StateBase::Goto( &StateCalibrateCenter::instance );
    }

    if ( TXInput::instance.isButtonPressed( LEFT_BUMPER_ID) || TXInput::instance.isButtonPressed( LEFT_TRIGGER_ID) )
    {
      TXInput::instance.finishAxisCenterCalibration();

      TXInput::instance.saveAxisCalibrationData();

      AudioManager::instance.play( "/calibration_finished.mp3", AUDIO_GROUP_NONE );
      AudioManager::instance.waitFinish();

      StateBase::Goto( &StateRun::instance );
    }
  }

  TXInput::instance.dumpADC();
}
