#include "StateRun.h"
#include "TXMain.h"
#include "TXInput.h"

#include "modeConfig.h"
#include "modeEspNowRC.h"
#include "modeBLEGamepad.h"

#include "AudioManager.h"
#include "ErrorLog.h"

StateRun StateRun::instance;

//======================================================
//======================================================
void StateRun::onEnter(StateBase *prevState)
{
  AudioManager::instance.sayProfile(currentProfileIndex);
  this->initLEDS4RunningLight();
  this->LEDS4RunningLightAudioWait();
  this->configModeNameAudioPlayed = false;
}

//======================================================
//======================================================
void StateRun::onRun(uint32_t t)
{
  HXChannels channelValues;
  TXInput::instance.getChannelValues( &channelValues );
  channelValues.channelValue[15] = ((int16_t)currentProfileIndex) * 100 + 1000;

//    Serial.printf( "%04d %04d %04d %04d %03d %03d\n", ADC[0]>>2, ADC[1]>>2, ADC[2]>>2, ADC[3]>>2, left_bumper_button &0xf, right_bumper_button &0xf );

  ModeBase::currentModeHandler->loop( &channelValues, &externalBTSerial, NULL );

  if ( !this->configModeNameAudioPlayed )
  {
    if ( ModeBase::currentModeHandler == &ModeConfig::instance )
    {
        AudioManager::instance.play( "/config_mode.mp3", AUDIO_GROUP_NONE );
        this->configModeNameAudioPlayed = true;
    }
  }

  if ( ModeBase::currentModeHandler == &ModeConfig::instance )
  {
    TXMain::instance.setLEDS4( (t % 1000) > 500 ?  8 + 4 + 2 + 1 : 0 ); 
  }
  else if ( ModeBase::currentModeHandler == &ModeIdle::instance )
  {
    this->SetLEDS4RunningLight();
  }
  else
  {
    this->SetLEDS4Profile(currentProfileIndex);
  }

  if ( ErrorLog::instance.getHasError())
  {
    AudioManager::instance.play( "/error.mp3", AUDIO_GROUP_NONE );
  }
}
