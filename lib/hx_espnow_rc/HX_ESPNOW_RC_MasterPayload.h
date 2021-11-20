#pragma once

#include "HX_ESPNOW_RC_Common.h"
#include "HX_ESPNOW_RC_Channels.h"

#define HXRC_MASTER_PAYLOAD_SIZE_BASE (4 + 2 + 2 + 2 + 2 + 22 + 1 )  
//#define HXRC_MASTER_TELEMETRY_SIZE_MAX ( HXRC_PAYLOAD_SIZE_MAX - HXRC_MASTER_PAYLOAD_SIZE_BASE )
#define HXRC_MASTER_TELEMETRY_SIZE_MAX 64  //limit packet size to improve chances of successfull delivery

#pragma pack (push)
#pragma pack (1)

class HXRCConfig;

//=====================================================================
//=====================================================================
typedef struct 
{
    //crc32 of the rest of a message
    uint32_t crc;  

    //transmitter and receiver should have same key
    uint16_t key;  

    //increments with each packet sent and each packet which should have been sent, but was not sent because
    //loop() function was called too late (master is required to keep specified minimum update rate)
    //master can count how many packets were missed and calculate RSSI
    uint16_t packetId;  

    //sequenceId increments with each acknowledged packet
    //sequenceId is used to implement stable telemetry stream,
    //and to calculate RSSI on master
    uint16_t sequenceId;

    //acknowledge of last incoming packet
    uint16_t ackSequenceId;

    HXRCChannels channels;

    uint8_t length;
    uint8_t data[HXRC_MASTER_TELEMETRY_SIZE_MAX];

    void setCRC();
    bool checkCRC() const;
} HXRCMasterPayload;

#pragma pack (pop)
