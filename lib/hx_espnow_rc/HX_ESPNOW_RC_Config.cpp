#include "HX_ESPNOW_RC_Config.h"
#include "HX_ESPNOW_RC_Common.h"

//=====================================================================
//=====================================================================
HXRCConfig::HXRCConfig()
{
    wifi_channel = 1;
    memset( this->peer_mac, 0xff, 6 );
    memset( this->key, 0, ESP_NOW_KEY_LEN);
    this->LRMode = false;
    this->ledPin = -1;
    this->ledPinInverted = false;
    this->keyCRC = 0;
}

//=====================================================================
//=====================================================================
HXRCConfig::HXRCConfig(
        uint8_t wifi_channel,
        uint8_t peer_mac[6],
        const char* key,  
        bool LRMode,
        int8_t ledPin,
        bool ledPinInverted
    )
{
    this->wifi_channel = wifi_channel;
    memcpy( this->peer_mac, peer_mac, 6 );
    memcpy( this->key, key, ESP_NOW_KEY_LEN );
    this->LRMode = LRMode;
    this->ledPin = ledPin;
    this-> ledPinInverted = ledPinInverted;

    this->setKeyCRC();
}

//=====================================================================
//=====================================================================
void HXRCConfig::setKeyCRC()
{
    this->keyCRC = 0;
    HXRC_crc16_update_buffer( &this->keyCRC, this->key, ESP_NOW_KEY_LEN );
}

