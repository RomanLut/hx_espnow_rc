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

    AudioManager::instance.play( "/calibrate_min_max.mp3", AUDIO_GROUP_NONE );
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
    if ( TXInput::instance.isAxisMinMaxCalibrationSuccessfull() ) 
    {
        StateBase::Goto(&StateCalibrateWaitCenter::instance);
    }
    else
    {
        StateBase::Goto(&StateCalibrateMinMax::instance);
    }
  }

  TXInput::instance.dumpADC();
}
