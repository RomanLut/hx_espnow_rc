#pragma once

#include <Arduino.h>
#include <stdint.h>

#define MAVLINK_RC_CHANNELS_COUNT_V1        8 //Mavlink v1 can handle 8 channels only      
#define MAVLINK_RC_CHANNELS_COUNT           16

//=====================================================================
//=====================================================================
class HXMavlinkRCEncoder
{
private:
    bool mavlink_v1;
    bool failsafe;
    uint16_t packetRateMS;
    uint16_t channels[MAVLINK_RC_CHANNELS_COUNT];
    unsigned long lastPacketTime;

    void initChannels();

public:
    HXMavlinkRCEncoder();

    //packetRateMS - write packet every ?ms
    void init( uint16_t packetRateMS, bool mavlink_v1 );

    void setFailsafe( bool failsafe );
    void setChannelValue( uint8_t index, uint16_t value );
    void loop( HardwareSerial& serial );
};



