#include "StateCalibrateMinMax.h"

#include "StateCalibrateWaitCenter.h"

#include "TXMain.h"
#include "TXInput.h"

StateCalibrateMinMax StateCalibrateMinMax::instance;

//======================================================
//======================================================
void StateCalibrateMinMax::onEnter()
{
    this->stateTime = millis();
    TXInput::instance.calibrateInitADC();
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
  
  TXInput::instance.calibrateAdjustMinMaxADC();

  if ( t - stateTime > 7000)
  {
    if ( TXInput::instance.isMinMaxCalibrationSuccessfull() ) 
    {
        StateBase::Goto(&StateCalibrateWaitCenter::instance);
    }
    else
    {
        StateBase::Goto(&StateCalibrateMinMax::instance);
    }
  }
}
