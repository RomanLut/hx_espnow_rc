#pragma once

#include <Arduino.h>
#include <stdint.h>

#include "HX_ESPNOW_RC_PromiscuousCapture.h"

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
//#include <esp_wifi_internal.h> 
#endif

#define HXRCLOG (*HXRCGetLogStream())

#define DEFAULT_PACKET_SEND_PERIOD_MS   20      //8(120Hz) is max for normal mode  
#define DEFAULT_PACKET_SEND_PERIOD_LR_MS   25      //25(40Hz)if max for LR mode
#define DEFAULT_FAILSAFE_PERIOD_MS      1000

#define HXRC_CHANNELS_COUNT 16

#define HXRC_TELEMETRY_BUFFER_SIZE   512

#define HXRC_PAYLOAD_SIZE_MAX 250

#define HXRC_PROTOCOL_VERSION 1

class HXRCConfig;

extern uint8_t BROADCAST_MAC[6];

#if defined(ESP32)
extern HXRCPromiscuousCapture capture;
#endif

//=====================================================================
//=====================================================================

extern Stream* HXRCGetLogStream();
void HXRCSetLogStream( Stream* value );

extern void HXRCInitLedPin( const HXRCConfig& config );
extern bool HXRCInitEspNow( HXRCConfig& config );

extern void HXRC_crc32_init();
extern uint32_t HXRC_crc32(const void* data, size_t length, uint32_t previousCrc32 = 0);

extern void HXRCPrintMac( const uint8_t* mac );