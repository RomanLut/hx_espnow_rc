#pragma once

#include "HX_ESPNOW_RC_Common.h"
#include "HX_ESPNOW_RC_Channels.h"

#define HXRC_MASTER_PAYLOAD_SIZE_BASE (2 + 2 + 22 + 1 )  //crc, sequenceId, channels, telemetry length
//#define HXRC_MASTER_TELEMETRY_SIZE_MAX ( HXRC_PAYLOAD_SIZE_MAX - HXRC_MASTER_PAYLOAD_SIZE_BASE )
#define HXRC_MASTER_TELEMETRY_SIZE_MAX 64  //limit packet size  to achieve target packet rate

#pragma pack (push)
#pragma pack (1)

class HXRCConfig;

//=====================================================================
//=====================================================================
typedef struct 
{
    uint16_t crc;

    //sequenceId increments with each sent packet
    //and each packet which was not sent in time ( transmitter is required to keep PACKET_SEND_PERIOD_MS miminum rate)
    uint16_t sequenceId;

    HXRCChannels channels;

    uint8_t length;
    uint8_t data[HXRC_MASTER_TELEMETRY_SIZE_MAX];

    void setCRC(HXRCConfig& config);
    bool checkCRC(HXRCConfig& config) const;
} HXRCMasterPayload;

#pragma pack (pop)
