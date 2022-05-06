#include <Arduino.h>
#include <AudioGenerator.h>

#ifndef _AUDIOGENERATORPOPKILLER_H
#define _AUDIOGENERATORPOPKILLER_H

class AudioGeneratorPopKiller : public AudioGenerator
{
 public:
    AudioGeneratorPopKiller( AudioGenerator* base, unsigned int rampMs);
    ~AudioGeneratorPopKiller() {};
    virtual bool begin(AudioFileSource *source, AudioOutput *output) override;
    virtual bool loop() override;
    virtual bool stop() override;
    virtual bool isRunning() override;

  protected:
    unsigned int rampMs;
    AudioGenerator* base;
    uint8_t state;
    unsigned int samples;    
    AudioOutput *output;

    unsigned int getRampSamplesCount();
};

#endif 