#include "AudioManager.h"

#include <esp_task_wdt.h>

static Audio audio(true, I2S_DAC_CHANNEL_RIGHT_EN);

 AudioManager AudioManager::instance;

//======================================================
//======================================================
void AudioManager::init()
{
  this->length = 0;
  audio.setVolume(21); // 0...21
  //audio.setTone(6,6,6);
  //audio.setBufsize( 50000,0);
}

//======================================================
//======================================================
bool AudioManager::loop( uint32_t t )
{
  if  ( this->length && !audio.isRunning() )
  {
    char fName[32];
    sprintf( fName, "%d.wav", this->queue[0].soundId);
    Serial.print("Play:");
    Serial.println(fName);
    audio.connecttoFS(SPIFFS, fName);
    this->removeItem(0);
  }
  audio.loop();

  return ( this->length || audio.isRunning() );
}

//======================================================
//======================================================
void AudioManager::waitFinish()
{
  while ( this->loop( millis()) )
  {
    esp_task_wdt_reset();
  }

}

//======================================================
//======================================================
void AudioManager::removeItem(int index)
{
  for ( int i = index + 1; i < this->length; i++ )
  {
    this->queue[i] = this->queue[i+1];
  }
  this->length--;
}

//======================================================
//======================================================
void AudioManager::removeSoundGroup(int soundGroup)
{
  for ( int i = 0; i < this->length; i++ )
  {
    if ( this->queue[i].soundGroup == soundGroup)
    {
      this->removeItem(i);
      return;
    }
  }
}

//======================================================
//======================================================
void AudioManager::play( uint8_t soundId, uint8_t soundGroup)
{
  if ( soundGroup != AUDIO_GROUP_NONE)
  {
    this->removeSoundGroup(soundGroup);
  }
  if ( this->length == AUDIO_QUEUE_LENGTH )
  {
    this->removeItem(0);
  }
  this->queue[this->length].soundGroup = soundGroup;
  this->queue[this->length].soundId = soundId;
  this->length++;
}


