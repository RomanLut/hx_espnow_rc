#pragma once

#include "HX_ESPNOW_RC_Common.h"

//=====================================================================
//=====================================================================
class HXChannels
{
private:

public:
    bool isFailsafe;
    //for ModeBase system, values are in range 1000...2000
    //for ModeBase system, channel 15 value is used to select active profile in rc transmitter
    //in rc link, channel 15 is used to send failsafe status ( no sbus pulses in external bay ) from rc transmitter to receiver
    int16_t channelValue[HXRC_CHANNELS_COUNT];  

    void dump()
    {
        Serial.print("Failsafe: ");
        Serial.println(this->isFailsafe?1: 0);

        for ( int i = 0; i < HXRC_CHANNELS_COUNT; i++ ) 
        {
            Serial.print("Channel");
            Serial.print(i);
            Serial.print(": ");
            Serial.println(this->channelValue[i]);
        }
    }
    
};


