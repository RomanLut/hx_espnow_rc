#pragma once

#include <Arduino.h>
#include <stdint.h>

#if defined(ESP8266)

#include <ESP8266WiFi.h>
#include <espnow.h>
#include <user_interface.h>

#define ESP_OK 0
#define esp_err_t int
//#define esp_now_send_status_t uint8_t
#define ESP_NOW_SEND_SUCCESS 0

#define ESP_NOW_KEY_LEN 16

#elif defined(ESP32)

#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

#endif

#define HXRC_VERSION 1

#define PACKET_SEND_PERIOD_MS   100      //10Hz    
#define FAILSAFE_PERIOD_MS      1000

#define HXRC_CHANNELS 16

#define HXRC_TELEMETRY_BUFFER_SIZE   512

//=====================================================================
//=====================================================================
class HXRCConfig
{
public:
    uint8_t wifi_channel;
    uint8_t peer_mac[6];
    uint8_t key[ESP_NOW_KEY_LEN];
    bool LRMode;
    int8_t ledPin;
    int8_t ledPinInverted;

    HXRCConfig()
    {
        wifi_channel = 1;
        memset( this->peer_mac, 0xff, 6 );
        memset( this->key, 0, ESP_NOW_KEY_LEN);
        this->LRMode = false;
        this->ledPin = -1;
        this->ledPinInverted = false;
    }

    HXRCConfig(
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
    }
};

#pragma pack (push)
#pragma pack (1)

//=====================================================================
//=====================================================================

#define HXRC_PAYLOAD_SIZE_MAX 250

#define HXRC_MASTER_PAYLOAD_SIZE_BASE (2 + 22 + 1 )  //sequenceId, channels, telemetry length
//#define HXRC_MASTER_TELEMETRY_SIZE_MAX ( HXRC_PAYLOAD_SIZE_MAX - HXRC_MASTER_PAYLOAD_SIZE_BASE )
#define HXRC_MASTER_TELEMETRY_SIZE_MAX 64  //limit packet size  to achieve target packet rate

typedef struct 
{
    uint16_t ch1        : 11; 
    uint16_t ch2        : 11;
    uint16_t ch3        : 11;
    uint16_t ch4        : 11;
    uint16_t ch5        : 11;
    uint16_t ch6        : 11;
    uint16_t ch7        : 11;
    uint16_t ch8        : 11;
    uint16_t ch9        : 11;
    uint16_t ch10       : 11;
    uint16_t ch11       : 11;
    uint16_t ch12       : 11;
    uint16_t ch13       : 11;
    uint16_t ch14       : 11;
    uint16_t ch15       : 11;
    uint16_t ch16       : 11;   //16*11 = 22 bytes

    void init();
    uint16_t getChannelValue( uint8_t index ) const;
    void setChannelValue(uint8_t index, uint16_t data);
} HXRCChannels;


//=====================================================================
//=====================================================================
typedef struct 
{
    //sequenceId increments with each sent packet
    //and each packet which was not sent in time ( transmitter is required to keep PACKET_SEND_PERIOD_MS miminum rate)
    uint16_t sequenceId;

    HXRCChannels channels;

    uint8_t length;
    uint8_t data[HXRC_MASTER_TELEMETRY_SIZE_MAX];
} HXRCPayloadMaster;

//=====================================================================
//=====================================================================

#define HXRC_SLAVE_PAYLOAD_SIZE_BASE (2 + 1 + 1 )  //sequenceId, rssi, telemetry length
#define HXRC_SLAVE_TELEMETRY_SIZE_MAX ( HXRC_PAYLOAD_SIZE_MAX - HXRC_SLAVE_PAYLOAD_SIZE_BASE )

typedef struct 
{
    uint16_t sequenceId;

    int8_t rssi;

    uint8_t length;
    uint8_t data[HXRC_SLAVE_TELEMETRY_SIZE_MAX];
} HXRCPayloadSlave;

#pragma pack (pop)

//=====================================================================
//=====================================================================
typedef enum
{
    HXRCSS_READY_TO_SEND        = 0,
    HXRCSS_RETRY_SEND           = 1,
    HXRCSS_WAITING_CONFIRMATION = 2
} HXRCSenderStateEnum;

//=====================================================================
//=====================================================================
extern void HXRCInitLedPin( const HXRCConfig& config );
extern bool HXRCInitEspNow( HXRCConfig& config );
