#include "StateCalibrateMinMax.h"

#include "StateCalibrateWaitCenter.h"

#include "AudioManager.h"

#include "TXMain.h"
#include "TXInput.h"

StateCalibrateMinMax StateCalibrateMinMax::instance;

//======================================================
//======================================================
void StateCalibrateMinMax::onEnter(StateBase *prevState)
{
    this->stateTime = millis();
    if ( prevState != &StateCalibrateMinMax::instance )
    {
      TXInput::instance.calibrateAxisInitADC();
    }

    AudioManager::instance.play( (count & 1 ) == 0 ? "/calibrate_min_max.mp3" : "/lb_finish.mp3", AUDIO_GROUP_NONE );
    this->count++;
}

//======================================================
//======================================================
void StateCalibrateMinMax::onRun(uint32_t t)
{
  if ( (t % 1000) < 500 )
  {
    TXMain::instance.setLEDS4( 4 + 2 ); 
  }
  else
  {
    TXMain::instance.setLEDS4( 8 + 1 ); 
  }

  TXInput::instance.loop(t);
  
  TXInput::instance.calibrateAxisAdjustMinMaxADC();

  if ( t - stateTime > 7000)
  {
    TXInput::instance.dumpAxisMinMaxCalibration();
    StateBase::Goto(&StateCalibrateMinMax::instance);
  }

  if ( TXInput::instance.isButtonPressed( LEFT_BUMPER_ID) || TXInput::instance.isButtonPressed( LEFT_TRIGGER_ID) )
  {
    TXInput::instance.finishAxisMinMaxCalibration();
    StateBase::Goto(&StateCalibrateWaitCenter::instance);
  }

  TXInput::instance.dumpADC();
}
