#pragma once

#include "HX_ESPNOW_RC_Common.h"

#define HXRC_SLAVE_PAYLOAD_SIZE_BASE (2 + 2 + 1 + 1 )  //crc, sequenceId, rssi, telemetry length
#define HXRC_SLAVE_TELEMETRY_SIZE_MAX ( HXRC_PAYLOAD_SIZE_MAX - HXRC_SLAVE_PAYLOAD_SIZE_BASE )

#pragma pack (push)
#pragma pack (1)

class HXRCConfig;

//=====================================================================
//=====================================================================
typedef struct 
{
    uint16_t crc; 

    uint16_t sequenceId;

    int8_t rssi;

    uint8_t length;
    uint8_t data[HXRC_SLAVE_TELEMETRY_SIZE_MAX];

    void setCRC(HXRCConfig& config);
    bool checkCRC(HXRCConfig& config) const;
} HXRCSlavePayload;

#pragma pack (pop)

