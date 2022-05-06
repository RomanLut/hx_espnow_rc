#include "AudioGeneratorPopKiller.h"


AudioGeneratorPopKiller::AudioGeneratorPopKiller( AudioGenerator* base, unsigned int rampMs)
{
    this->base = base;
    this->rampMs = rampMs;
    this->state = 0;
}


bool AudioGeneratorPopKiller::begin(AudioFileSource *source, AudioOutput *output) 
{
    if (  this->base->begin( source, output ) )
    {
        this->output = output;
        this->state = 1;
        this->samples = 0;

        this->output->SetRate( 24000);
        return true;
    }

    return false;
}

bool AudioGeneratorPopKiller::loop() 
{
    if ( state == 1 )
    {
        unsigned int count = getRampSamplesCount(); 
        while( true )
        {
            if ( samples > count )
            {
                state = 2;
                break;
            }
            else
            {
                //-32768 - no pop on start
                int16_t sample[2];
                
                //sample[0] = -(0x8000) + (uint32_t)0x8000 * samples / count;
                //if ( output->GetBitsPerSample() == 8 ) sample[0] >>= 8;
                
                //sample[0] = -(0x80) + (uint32_t)0x80 * samples / count;
                //if ( output->GetBitsPerSample() == 16 ) sample[0] <<= 8;

                sample[0] = -(0x8000) + (uint32_t)0x8000 * samples / count;
                sample[0] &= 0xf800; //use large steps to avoid aliasing with PDM
                if ( output->GetBitsPerSample() == 8 ) sample[0] >>= 8;

                sample[1] = sample[0];
                if ( output->ConsumeSample(sample) )
                {
                    samples++;
                }
                else
                {
                    return true;
                }
            }
        }
       
    }

    if ( state == 3 )
    {
        unsigned int count = getRampSamplesCount(); 
        while( true )
        {
            if ( samples > count )
            {
                state = 4;
            Serial.println("state4");
                return true;
            }
            else
            {
                int16_t sample[2];
                
                sample[0] = -((uint32_t)0x8000 * samples / count);
                sample[0] &= 0xf800; //use large steps to avoid aliasing with PDM
                if ( output->GetBitsPerSample() == 8 ) sample[0] >>= 8;

                sample[1] = sample[0];
                if ( output->ConsumeSample(sample) )
                {
                    samples++;
                }
                else
                {
                    return true;
                }
            }
        }
       
    }

    if ( state == 4 )
    {
        unsigned int count = 128*8*2; //buffers count;
        while( true )
        {
            if ( samples > count )
            {
                state = 0;
            Serial.println("state0");
                return true;
            }
            else
            {
                int16_t sample[2];
                
                sample[0] = -32768;
                if ( output->GetBitsPerSample() == 8 ) sample[0] >>= 8;

                sample[1] = sample[0];
                if ( output->ConsumeSample(sample) )
                {
                    samples++;
                }
                else
                {
                    return true;
                }
            }
        }
       
    }

    bool b = this->base->loop();
    if ( !b && state == 2)
    {
        state = 3;
        samples = 0;
        Serial.println("state3");
        return true;            
    }
    return b;
}

bool AudioGeneratorPopKiller::stop() 
{
    state = 0;
    return this->base->stop();
}

bool AudioGeneratorPopKiller::isRunning() 
{
    if ( state == 1 ) return true;
    if ( state == 2 ) return true;
    if ( state == 3 ) return true;            
    if ( state == 4 ) return true;            
    return this->base->isRunning();
}

unsigned int AudioGeneratorPopKiller::getRampSamplesCount() 
{
    uint32_t v = (uint32_t)this->output->GetRate() * this->rampMs / 1000;
    return v > 65000 ? 65000 : v;
}



