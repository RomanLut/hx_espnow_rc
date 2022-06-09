#include "StateBase.h"
#include "TXMain.h"

#include "AudioManager.h"

StateBase* StateBase::currentState = NULL;

uint32_t StateBase::lastProfileMessage = 0;

//======================================================
//======================================================
void StateBase::Goto(StateBase* s)
{
  StateBase* prev = StateBase::currentState;
  StateBase::currentState = s;
  StateBase::currentState->onEnter(prev);
}

//======================================================
//======================================================
void StateBase::SetLEDS4Profile(uint8_t profileIndex)
{
  switch ( profileIndex)
  {
    case 0:
      TXMain::instance.setLEDS4( 8 );
      break;

    case 1:
      TXMain::instance.setLEDS4( 4 );
      break;

    case 2:
      TXMain::instance.setLEDS4( 2 );
      break;

    case 3:
      TXMain::instance.setLEDS4( 1 );
      break;

    case 4:
      TXMain::instance.setLEDS4( 4 + 2 + 1 );
      break;

    case 5:
      TXMain::instance.setLEDS4( 8 + 2 + 1 );
      break;

    case 6:
      TXMain::instance.setLEDS4( 8 + 4 + 1 );
      break;

    case 7:
      TXMain::instance.setLEDS4( 8 + 4 + 2 );
      break;
  }
}

//======================================================
//======================================================
void StateBase::initLEDS4RunningLight()
{
    this->runningLightStart = millis();
}

//======================================================
//======================================================
void StateBase::SetLEDS4RunningLight()
{
    int phase = ((millis() - this->runningLightStart) % 696) / 116;
    static const uint8_t pattern[] = {8,4,2,1,2,4};
    TXMain::instance.setLEDS4( pattern[phase] );
}

//======================================================
//======================================================
void StateBase::LEDS4RunningLightAudioWait()
{
  while ( AudioManager::instance.loop( millis() ))
  {
      esp_task_wdt_reset();
      this->SetLEDS4RunningLight();
  }
    TXMain::instance.setLEDS4( 0 );
}