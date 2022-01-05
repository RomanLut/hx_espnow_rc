#pragma once

#include <Arduino.h>
#include <stdint.h>

#if defined(ESP32)
#include <esp_wifi_types.h>

//=====================================================================
//=====================================================================
typedef struct
{
  unsigned frame_ctrl : 16;  // 2 bytes / 16 bit fields
  unsigned duration_id : 16; // 2 bytes / 16 bit fields
  uint8_t addr1[6];          // receiver address
  uint8_t addr2[6];          //sender address
  uint8_t addr3[6];          // filtering address
  unsigned sequence_ctrl : 16; // 2 bytes / 16 bit fields
} wifi_ieee80211_mac_hdr_t;    // 24 bytes

//=====================================================================
//=====================================================================
typedef struct
{
  wifi_ieee80211_mac_hdr_t hdr;
  uint8_t payload[0];
} wifi_ieee80211_packet_t;

//=====================================================================
//=====================================================================
class HXRCPromiscuousCapture
{
public:
    uint8_t peerMac[6];
    int8_t rssi;   //Received Signal Strength Indicator(RSSI) of packet. unit: dBm
    int8_t noiseFloor; //noise floor of Radio Frequency Module(RF). unit: 0.25dBm. In practice: in dbm
    int rate;
    uint32_t packetsCount;

    HXRCPromiscuousCapture()
    {
        memset( peerMac, 0, 8);
        rssi = 0;
        noiseFloor = 0;
        rate = -1;
    }
};
#endif

//about RSSI levels: 
//https://www.netspotapp.com/what-is-rssi-level.html
//https://esp32.com/viewtopic.php?t=3096
//https://circuitdigest.com/microcontroller-projects/esp32-based-bluetooth-ibeacon
//esp32 sensitivity: https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
//https://esp32.com/viewtopic.php?t=3096