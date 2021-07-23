#pragma once

#include <Arduino.h>
#include <stdint.h>

#include "HX_ESPNOW_RC_Common.h"

//=====================================================================
//=====================================================================
class HXRCConfig
{
public:
    uint8_t wifi_channel;
    bool LRMode;
    int8_t ledPin;
    bool ledPinInverted;
    uint16_t key;

    HXRCConfig();

    HXRCConfig(
        uint8_t wifi_channel,
        uint16_t key,
        bool LRMode,
        int8_t ledPin,
        bool ledPinInverted
    );
    
};
