#pragma once

#include <Arduino.h>

#include <SPIFFS.h>

#include "AudioFileSourceSPIFFS.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

#include <stdio.h>
#include <stdarg.h>

#include "tx_config.h"

#define AUDIO_QUEUE_LENGTH      3

#define AUDIO_GROUP_NONE        0      
#define AUDIO_GROUP_PROFILE     1
#define AUDIO_GROUP_SOUND       2

/*
Sounds
============
voicemaker.in
Neural TSS
Salli, Female TSS
WAV 16000
Speed 19%
DRC
============
0 intro beeps
1 Sticks calibration started. Move all sticks to minimum and maximum positions.
2 Move all sticks to center positions.
3 Sticks calibration successfull.
4 Profile selection mode started. 
5 Profile 1 selected.
6 Profile 2 selected.
7 Profile 3 selected.
8 Profile 4 selected.
Profile 5 selected.
Profile 6 selected.
Profile 7 selected.
Profile 8 selected.
Configuration mode started.
Battery 100 percent.
Battery 75 percent.
Battery 50 percent.
Battery 25 percent.
Controller battery low. Please charge.
Sound off. 
Sound on.
*/

typedef struct
{
    String fileName;
    uint8_t soundGroup;
} AudioItem;

//=====================================================================
//=====================================================================
class AudioManager
{
private:
    AudioGeneratorMP3* mp3 = NULL;
    AudioFileSourceSPIFFS* file = NULL;
    AudioOutputI2S* output = NULL;        

    AudioItem queue[AUDIO_QUEUE_LENGTH];
    uint8_t length;

    void removeItem(int index);
    void removeSoundGroup(int soundGroup);
//    void prefetch( const char* fileName );
    
public:

    static AudioManager instance;

    void init();
    bool loop( uint32_t t );
    void waitFinish();

    void play( String fileName, uint8_t soundGroup);

};

