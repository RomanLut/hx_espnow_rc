#pragma once

#include <Arduino.h>
#include <stdint.h>

#include "HX_ESPNOW_RC_Common.h"

//=====================================================================
//=====================================================================
class HXRCConfig
{
public:

    typedef enum {
        PACKET_RATE_DEFAULT = 0,
        PACKET_RATE_MAX = 0xfff
    } PacketRateEnum;

#if defined(ESP32)
    typedef enum {
        WIFI_PHY_RATE_DEFAULT= 0xff
    } PhyRateEnum;
#endif 

    typedef enum {
        SLAVE_TELEMETRY_PAYLOAD_SIZE_DEFAULT = 0
    } SlaveTelemtryPayloadSizeEnum;

    uint8_t wifi_channel;
    int8_t ledPin;
    bool ledPinInverted;
    uint16_t key;

    //set on transmitter side. Period between master packets (packet rate = 1000ms / packetRatePeriodMS )
    uint16_t packetRatePeriodMS = PACKET_RATE_DEFAULT;

#if defined(ESP32)
    bool LRMode;

    //PhyRateEnum or wifi_phy_rate_t
    //should be the same on both ends
    //For LR mode valid rates are: WIFI_PHY_RATE_LORA_250K, WIFI_PHY_RATE_LORA_500K (and these rates are not valid for normal mode)
    uint16_t wifiPhyRate = WIFI_PHY_RATE_DEFAULT;
#endif 

    //size of telemery payload on slave
    //affects message length in us which is crtical in LR mode
    //can be increased up to HXRC_SLAVE_TELEMETRY_SIZE_MAX to increase telemtry throughput at the cost of range
    uint8_t slaveTelemertyPayloadSize = SLAVE_TELEMETRY_PAYLOAD_SIZE_DEFAULT;

    HXRCConfig();

    HXRCConfig(
        uint8_t wifi_channel,
        uint16_t key,
#if defined(ESP32)
        bool LRMode,
#endif 
        int8_t ledPin,
        bool ledPinInverted
    );

    uint16_t getDefaultPacketRatePeriodMS();
    uint16_t getDesiredPacketRatePeriodMS();
    int getSlaveTelemetryPayloadSize();

#if defined(ESP32)
    int getDefaultWifiPhyRate();
    int getDesiredWifiPhyRate();
#endif 
};
