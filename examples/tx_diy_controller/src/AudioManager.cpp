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
void AudioManager::prefetch( const char* fileName )
{
    File file = SPIFFS.open(fileName);
    if  (!file) return;

    char buffer[1024];

    while( file.available() )
    {
      file.readBytes( buffer, 1024);
    }

    file.close();
}

//======================================================
//======================================================
bool AudioManager::loop( uint32_t t )
{
  if  ( this->length && !audio.isRunning() )
  {
    Serial.print("Play:");
    Serial.println(this->queue[0].fileName);
    this->prefetch(this->queue[0].fileName.c_str());
    audio.connecttoFS(SPIFFS, this->queue[0].fileName.c_str());
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
void AudioManager::play( String fileName, uint8_t soundGroup)
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
  this->queue[this->length].fileName = fileName;
  this->length++;
}


