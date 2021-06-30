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

#define DEFAULT_PACKET_SEND_PERIOD_MS   100      //10Hz    
#define DEFAULT_FAILSAFE_PERIOD_MS      1000

#define HXRC_CHANNELS 16

#define HXRC_TELEMETRY_BUFFER_SIZE   512

#define HXRC_PAYLOAD_SIZE_MAX 250

//=====================================================================
//=====================================================================
typedef enum
{
    HXRCSS_READY_TO_SEND        = 0,
    HXRCSS_RETRY_SEND           = 1,
    HXRCSS_WAITING_CONFIRMATION = 2
} HXRCSenderStateEnum;

class HXRCConfig;

//=====================================================================
//=====================================================================
extern void HXRCInitLedPin( const HXRCConfig& config );
extern bool HXRCInitEspNow( HXRCConfig& config );
extern void HXRC_crc16_update( uint16_t* crc, uint8_t a );
extern void HXRC_crc16_update_buffer( uint16_t* crc, uint8_t* ptr, uint16_t len );

