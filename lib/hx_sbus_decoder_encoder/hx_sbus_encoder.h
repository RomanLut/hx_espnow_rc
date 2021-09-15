#pragma once

#include <Arduino.h>
#include <stdint.h>

#include "hx_sbus_packet.h"

//write packet every 11ms
#define SBUS_PERIOD_MS            11

//=====================================================================
//=====================================================================
class HXSBUSEncoder
{
private:
    HXSBUSPacket lastPacket;
    unsigned long lastPacketTime;

public:
    HXSBUSEncoder();

    void init( HardwareSerial& serial, uint8_t tx_pin, bool invert );

    void setFailsafe( bool failsafe );
    void setChannelValue( uint8_t index, uint16_t value );
    void loop( HardwareSerial& serial );
};



