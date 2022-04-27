#include "StateRun.h"
#include "TXMain.h"
#include "TXInput.h"

StateRun StateRun::instance;

//======================================================
//======================================================
void StateRun::onEnter()
{
}

//======================================================
//======================================================
void StateRun::onRun(uint32_t t)
{
  HXChannels channelValues;
  TXInput::instance.getChannelValues( &channelValues );
  channelValues.channelValue[15] = CH16;  

//    Serial.printf( "%04d %04d %04d %04d %03d %03d\n", ADC[0]>>2, ADC[1]>>2, ADC[2]>>2, ADC[3]>>2, left_bumper_button &0xf, right_bumper_button &0xf );

  ModeBase::currentModeHandler->loop( &channelValues, &externalBTSerial, NULL );

  const TXProfileManager* pProfile = TXProfileManager::getCurrentProfile();

  if ( pProfile && pProfile->transmitterMode == TM_CONFIG )
  {
    TXMain::instance.setLEDS4(  (t % 1000) > 500 ?  8 +4 +2 + 1 : 0); 
  }
  else if ( pProfile && pProfile->transmitterMode == TM_BLE_GAMEPAD )
  {
    TXMain::instance.setLEDS4( 8 ); 
  }
  else
  {
    TXMain::instance.setLEDS4( 8 + 4 + 2 + 1); 
  }
}
