#pragma once

#include <Arduino.h>
#include <stdint.h>

#include "hx_sbus_packet.h"

#define SBUS_SYNC_FAILSAFE_MS            200

//=====================================================================
//=====================================================================
class HXSBUSDecoder
{
private:
    //40 - wait for packet footer( sync)
    //30..39 - skip byte
    //0 - expect header
    //1..23 - receive body
    //24 expect footer
    //25 expect header
    uint8_t state;
    uint8_t syncCount;
    uint16_t packetsCount;
    uint8_t resyncCount;
    uint8_t resyncSkipCount;
    uint16_t failsafeCount;
    bool failsafeState;
    
    HXSBUSPacket packet;
    HXSBUSPacket lastPacket;
    unsigned long lastPacketTime;

    void parsePacket();
    void dumpPacket() const;
    void resync();
    void updateFailsafe();

public:
    HXSBUSDecoder();

    void init( int gpio );

    uint16_t getChannelValue( uint8_t index ) const;
    uint16_t getChannelValueInRange( uint8_t index, uint16_t from, uint16_t to ) const;
    bool isOutOfSync() const;
    bool isFailsafe() const;

    void loop();

    void dump() const;
};



