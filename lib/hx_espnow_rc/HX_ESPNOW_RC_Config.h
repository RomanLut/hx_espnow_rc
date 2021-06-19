#pragma once

#include <Arduino.h>
#include <stdint.h>

#include "HX_ESPNOW_RC_Common.h"

//=====================================================================
//=====================================================================
class HXRCConfig
{
private:
    void setKeyCRC();

public:
    uint8_t wifi_channel;
    uint8_t peer_mac[6];
    uint8_t key[ESP_NOW_KEY_LEN];
    bool LRMode;
    int8_t ledPin;
    int8_t ledPinInverted;

    uint16_t keyCRC;

    HXRCConfig();

    HXRCConfig(
        uint8_t wifi_channel,
        uint8_t peer_mac[6],
        const char* key,  
        bool LRMode,
        int8_t ledPin,
        bool ledPinInverted
    );
    
};
