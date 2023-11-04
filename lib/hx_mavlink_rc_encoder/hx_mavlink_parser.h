#pragma once

#include <Arduino.h>
#include <stdint.h>

#include <common/mavlink.h>

//=====================================================================
//=====================================================================
//This is helper class, a simplistic Mavlink packets parser, 
//which is used to find out if Mavlink packet transfer is in progress, 
//so we could inject our own packets in the steram
class HXMavlinkParser
{
private:

    typedef enum
    {
        MPS_IDLE,
        MPS_LENGTH,
        MPS_BODY
    } TMavParserState;
  
    bool v2;

    uint8_t state;
    int packetLength;
    int expectedLength;

    uint8_t sbuf[MAVLINK_MAX_PACKET_LEN]; 
    int sbufIndex;

    bool bGotPacket;

public:
    HXMavlinkParser(bool mavlinkV2);

    void processByte(uint8_t byte);

    bool gotPacket();
    const uint8_t* getPacketBuffer();
    int getPacketLength();
};



