#include "StateCalibrateCenter.h"

#include "StateRun.h"

#include "TXMain.h"
#include "TXInput.h"

StateCalibrateCenter StateCalibrateCenter::instance;

//======================================================
//======================================================
void StateCalibrateCenter::onEnter()
{
  this->stateTime = millis();

  TXInput::instance.calibrateInitADC2();
}

//======================================================
//======================================================
void StateCalibrateCenter::onRun(uint32_t t)
{
  TXMain::instance.setLEDS4( 4 + 2 ); 

  TXInput::instance.loop(t);
  TXInput::instance.calibrateAdjustMidMinMaxADC();

  if ( t - stateTime > 1000)
  {
    if ( TXInput::instance.isCenterCalibrationSuccessfull() ) 
    {
      TXInput::instance.saveCalibrationData();
      StateBase::Goto( &StateRun::instance );
    }
    else
    {
      StateBase::Goto( &StateCalibrateCenter::instance );
    }
  }
}
