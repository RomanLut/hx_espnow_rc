#include "HX_ESPNOW_RC_Common.h"
#include "HX_ESPNOW_RC_Config.h"

//#define COEXIST

#ifdef COEXIST
#include <esp_coexist.h>
#endif

uint8_t BROADCAST_MAC[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

#define CRC32_POLYNOMIAL 0xEDB88320
static uint32_t Crc32Lookup[256];

static Stream* HXRCLOGStream = NULL;

//=====================================================================
//=====================================================================
Stream* HXRCGetLogStream()
{
    return HXRCLOGStream ? HXRCLOGStream : &Serial;
}

//=====================================================================
//=====================================================================
void HXRCSetLogStream( Stream* value )
{
    HXRCLOGStream = value;
}

//=====================================================================
//=====================================================================
void HXRCInitLedPin( const HXRCConfig& config )
{
    if ( config.ledPin != -1 )
    {
        pinMode(config.ledPin,OUTPUT);
        digitalWrite(config.ledPin, config.ledPinInverted ? HIGH : LOW );
    }
}


//=====================================================================
//=====================================================================
bool HXRCInitEspNow( HXRCConfig& config )
{
#if defined(ESP8266)
    
    WiFi.mode(WIFI_STA);

    //promiscous mode is required to set channel
    wifi_promiscuous_enable(true);

    //Channel setting works only in WIFI_STA mode. Mode can be changed later.Channel will be preserved.
    if  (!wifi_set_channel( config.wifi_channel ) )
    {
        Serial.println("HXRC: Error: Failed to set channel");
        return false;
    }
    wifi_promiscuous_enable(false);

    //review: do we need to disable wifi sleep somehow?
    //esp_wifi_set_ps(WIFI_PS_NONE);

    Serial.print("HXESPNOWRC: Info: Board MAC address(STA): ");
    Serial.println(WiFi.macAddress());
    Serial.print("HXESPNOWRC: Info: Board MAC address(AP): ");
    Serial.println(WiFi.softAPmacAddress());

    if (esp_now_init() != ESP_OK)
    {
        Serial.println("HXESPNOWRC: Error: Error initializing ESP-NOW");
        return false;
    }

    if ( esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER) != ESP_OK )
    {
        Serial.println("HXRC: Error: Failed to set role");
        return false;
    }

    if ( esp_now_add_peer(BROADCAST_MAC, ESP_NOW_ROLE_COMBO, config.wifi_channel, NULL, 0) != ESP_OK )
    {
        Serial.println("HXRC: Error: Failed to add peer");
        return false;
    }

#elif defined(ESP32)

    WiFi.mode(WIFI_STA);

    esp_wifi_set_promiscuous(true); //promiscous mode is required to set channel on older SDK
    if ( esp_wifi_set_channel( config.wifi_channel, WIFI_SECOND_CHAN_NONE) != ESP_OK )
    {
        Serial.println("HXRC: Error: Failed to set channel");
        return false;
    }
    esp_wifi_set_promiscuous(false); 

    if ( esp_wifi_set_protocol (WIFI_IF_STA, config.LRMode ? WIFI_PROTOCOL_LR : ( WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N ) ) != ESP_OK)
    {
        Serial.println("HXRC: Error: Failed to enable LR mode");
        return false;
    }

    /*
#include "esp_private/wifi.h"
ESP_ERROR_CHECK(esp_wifi_start());
wifi_interface_t ifx = WIFI_IF_AP;
ESP_ERROR_CHECK(esp_wifi_internal_set_fix_rate(ifx, true, WIFI_PHY_RATE_1M_L));

WIFI_PHY_RATE_LORA_250K

If you want to use esp_wifi_internal_set_fix_rate, please disable WiFi AMPDU TX by:

make menuconfig => components => Wi-Fi => Disable TX AMPDU.
*/
    //Note: Wifi sleep should not be disabled if Wifi and Bluetooth coexist
    //https://github.com/espressif/arduino-esp32/issues/4965
    //https://github.com/espressif/esp-idf/issues/5759
    if ( esp_wifi_set_ps(WIFI_PS_MIN_MODEM) != ESP_OK )
    {
        Serial.println("HXRC: Error: Failed to set power mode");
        return false;
    }

    Serial.print("HXESPNOWRC: Info: Board MAC address(STA): ");
    Serial.println(WiFi.macAddress());
    Serial.print("HXESPNOWRC: Info: Board MAC address(AP): ");
    Serial.println(WiFi.softAPmacAddress());

    if ( esp_now_init() != ESP_OK )
    {
        Serial.println("HXESPNOWRC: Error: Error initializing ESP-NOW");
        return false;
    }

    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, BROADCAST_MAC, 6);
    peerInfo.channel = config.wifi_channel;
    memset(peerInfo.lmk, 0, ESP_NOW_KEY_LEN);
    peerInfo.encrypt = false;
    peerInfo.ifidx = WIFI_IF_STA;

    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("HXRC: Error: Failed to add peer");
        return false;
    }

#ifdef COEXIST
  if ( esp_coex_preference_set(ESP_COEX_PREFER_WIFI) != ESP_OK )
  {
    Serial.println("An error occurred initializing coexist setting");
  }
#endif


#endif

    return true;
}

//=====================================================================
//=====================================================================
void HXRC_crc32_init()
{
    for (unsigned int i = 0; i <= 0xFF; i++)
    {
        uint32_t crc = i;
        for (unsigned int j = 0; j < 8; j++)
            crc = (crc >> 1) ^ (-int(crc & 1) & CRC32_POLYNOMIAL);
        Crc32Lookup[i] = crc;
    }
}

//=====================================================================
//=====================================================================
uint32_t HXRC_crc32(const void* data, size_t length, uint32_t previousCrc32)
{
  uint32_t crc = ~previousCrc32;
  unsigned char* current = (unsigned char*) data;
  while (length--)
    crc = (crc >> 8) ^ Crc32Lookup[(crc & 0xFF) ^ *current++];
  return ~crc;
}