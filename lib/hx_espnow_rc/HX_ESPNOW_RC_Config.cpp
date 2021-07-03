#include "HX_ESPNOW_RC_Config.h"
#include "HX_ESPNOW_RC_Common.h"

//=====================================================================
//=====================================================================
HXRCConfig::HXRCConfig()
{
    this->wifi_channel = 1;
    this->key = 0;
    this->LRMode = false;
    this->ledPin = -1;
    this->ledPinInverted = false;
}

//=====================================================================
//=====================================================================
HXRCConfig::HXRCConfig(
        uint8_t wifi_channel,
        uint16_t key,
        bool LRMode,
        int8_t ledPin,
        bool ledPinInverted
    )
{
    this->wifi_channel = wifi_channel;
    this->key = key;
    this->LRMode = LRMode;
    this->ledPin = ledPin;
    this-> ledPinInverted = ledPinInverted;
}

