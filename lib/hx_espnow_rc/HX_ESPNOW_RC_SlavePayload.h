#pragma once

#include "HX_ESPNOW_RC_Common.h"

#define HXRC_SLAVE_PAYLOAD_SIZE_BASE (4 + 2 + 2 + 2 + 2 + 1 ) 
//#define HXRC_SLAVE_TELEMETRY_SIZE_MAX ( HXRC_PAYLOAD_SIZE_MAX - HXRC_SLAVE_PAYLOAD_SIZE_BASE )
#define HXRC_SLAVE_TELEMETRY_SIZE_MAX 128

#pragma pack (push)
#pragma pack (1)

class HXRCConfig;

//=====================================================================
//=====================================================================
typedef struct 
{
    uint32_t crc; 

    uint16_t key;

    uint16_t packetId;
    uint16_t sequenceId;
    uint16_t ackSequenceId;

    uint8_t length;
    uint8_t data[HXRC_SLAVE_TELEMETRY_SIZE_MAX];

    void setCRC();
    bool checkCRC() const;
} HXRCSlavePayload;

#pragma pack (pop)

