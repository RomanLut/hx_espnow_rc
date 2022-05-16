#include "StateInit.h"

#include "StateCalibrateMinMax.h"
#include "StateSelectProfile.h"
#include "StateRun.h"

#include "TXMain.h"
#include "TXInput.h"

#include "AudioManager.h"

StateInit StateInit::instance;

//======================================================
//======================================================
void StateInit::onEnter()
{
  this->stateTime = millis();  

  AudioManager::instance.play( "/intro.mp3", AUDIO_GROUP_NONE );
}

//======================================================
//======================================================
void StateInit::onRun(uint32_t t)
{
  int phase = (t - stateTime) / 500;

  TXInput::instance.loop(t);

  switch (phase)
  {
    case 0: 
      TXMain::instance.setLEDS4( 8 ); 
      break;
    case 1: 
      TXMain::instance.setLEDS4( 8 + 4); 
      break;
    case 2: 
      TXMain::instance.setLEDS4( 8 + 4 + 2); 
      break;
    case 3: 
      TXMain::instance.setLEDS4( 8 + 4 + 2 + 1 ); 
      break;
     case 4: 
      if ( TXInput::instance.isCalibrateGesture() )
      {
        AudioManager::instance.play( "/calibration.mp3", AUDIO_GROUP_NONE );
        AudioManager::instance.waitFinish();
        StateBase::Goto(&StateCalibrateMinMax::instance);
      }
      else if ( TXInput::instance.isButtonPressed(RIGHT_BUMPER_ID))
      {
        currentProfileIndex = CONFIG_PROFILE_INDEX;  //Mode Config profile
        StateBase::Goto(&StateRun::instance);
      }
      else if ( TXInput::instance.isButtonPressed(LEFT_BUMPER_ID))
      {
        StateBase::Goto(&StateSelectProfile::instance);
      }
      else
      {
        StateBase::Goto(&StateRun::instance);
      }
      break;
  }
}
