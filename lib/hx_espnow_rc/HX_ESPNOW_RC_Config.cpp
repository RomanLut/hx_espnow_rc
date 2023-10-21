#include "HX_ESPNOW_RC_Config.h"
#include "HX_ESPNOW_RC_Common.h"

//=====================================================================
//=====================================================================
HXRCConfig::HXRCConfig()
{
    this->wifi_channel = 1;
    this->key = 0;
#if defined(ESP32)
    this->LRMode = false;
#endif        
    this->ledPin = -1;
    this->ledPinInverted = false;
}

//=====================================================================
//=====================================================================
HXRCConfig::HXRCConfig(
        uint8_t wifi_channel,
        uint16_t key,
#if defined(ESP32)
        bool LRMode,
#endif        
        int8_t ledPin,
        bool ledPinInverted
    )
{
    this->wifi_channel = wifi_channel;
    this->key = key;
#if defined(ESP32)
    this->LRMode = LRMode;
#endif    
    this->ledPin = ledPin;
    this-> ledPinInverted = ledPinInverted;
}


//=====================================================================
//=====================================================================
uint16_t HXRCConfig::getDefaultPacketRatePeriodMS()
{
#if defined(ESP32)
    return this->LRMode ? DEFAULT_PACKET_SEND_PERIOD_LR_MS : DEFAULT_PACKET_SEND_PERIOD_MS;
#elif defined(ESP8266)
    return DEFAULT_PACKET_SEND_PERIOD_MS;
#endif    
}

//=====================================================================
//=====================================================================
uint16_t HXRCConfig::getDesiredPacketRatePeriodMS()
{
#if defined(ESP32)
    if ( this->packetRatePeriodMS == HXRCConfig::PACKET_RATE_MAX ) return this->LRMode ? 20: 5;  //todo: 1.5x period of expected packet rate for selected phy rate and client payload size
#elif defined(ESP8266)
    if ( this->packetRatePeriodMS == HXRCConfig::PACKET_RATE_MAX ) return 5;
#endif    
    return ( this->packetRatePeriodMS == HXRCConfig::PACKET_RATE_DEFAULT ) ? this->getDefaultPacketRatePeriodMS() : this->packetRatePeriodMS;
}

//=====================================================================
//=====================================================================
int HXRCConfig::getSlaveTelemetryPayloadSize()
{
    if ( this->slaveTelemertyPayloadSize == SLAVE_TELEMETRY_PAYLOAD_SIZE_DEFAULT)
    {
#if defined(ESP32)
        return this->LRMode ? 64 : 128;
#elif defined(ESP8266)
    return 128;
#endif    
    }
    else
    {
        return this->slaveTelemertyPayloadSize;
    }
}

#if defined(ESP32)
//=====================================================================
//=====================================================================
int HXRCConfig::getDefaultWifiPhyRate()
{
    return this->LRMode ? WIFI_PHY_RATE_LORA_250K : WIFI_PHY_RATE_1M_L;
}
#endif

#if defined(ESP32)
//=====================================================================
//=====================================================================
int HXRCConfig::getDesiredWifiPhyRate()
{
    int rate = this->getDefaultWifiPhyRate();
    return this->wifiPhyRate == WIFI_PHY_RATE_DEFAULT ? rate : wifiPhyRate;
}
#endif

