#include "AudioManager.h"

#include <esp_task_wdt.h>

 AudioManager AudioManager::instance;

//======================================================
//======================================================
void AudioManager::init()
{
  this->length = 0;
}

/*
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
*/

//======================================================
//======================================================
bool AudioManager::loop( uint32_t t )
{
  if  ( this->length && !mp3 )
  {
    Serial.print("Play:");
    Serial.println(this->queue[0].fileName);
//    this->prefetch(this->queue[0].fileName.c_str());

    this->file = new AudioFileSourceSPIFFS( this->queue[0].fileName.c_str() );
    this->output = new AudioOutputI2S(0, AudioOutputI2S::INTERNAL_DAC, 8, AudioOutputI2S::APLL_DISABLE);
    this->output->SetGain(2.0f);
    this->mp3 = new AudioGeneratorMP3();
    this->mp3->begin(file, output);

    this->removeItem(0);
  }

  if (this->mp3)
  {
    if ( this->mp3->isRunning() )
    {
       if (!mp3->loop()) 
       {
         mp3->stop();
          Serial.println("AudioStop");
       }
    }
    else
    {
      delete this->mp3;
      this->mp3 = NULL;

      delete this->file;
      this->file = NULL;
      
      delete this->output;
      this->output = NULL;        
    }
  } 

  return ( this->length || !!mp3 );
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


