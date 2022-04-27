#include "StateCalibrateWaitCenter.h"

#include "StateCalibrateCenter.h"

#include "TXMain.h"
#include "TXInput.h"

StateCalibrateWaitCenter StateCalibrateWaitCenter::instance;

//======================================================
//======================================================
void StateCalibrateWaitCenter::onEnter()
{
  this->stateTime = millis();

  TXInput::instance.calibrateInitADC2();
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
  TXInput::instance.calibrateAdjustMidMinMaxADC();

  if ( t - stateTime > 1000)
  {
    if ( TXInput::instance.isCenterCalibrationSuccessfull() ) 
    {
      StateBase::Goto( &StateCalibrateCenter::instance );
    }
    else
    {
      StateBase::Goto( &StateCalibrateWaitCenter::instance );
    }
  }

}
