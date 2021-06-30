#include "HX_ESPNOW_RC_Common.h"
#include "HX_ESPNOW_RC_Config.h"


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
    uint8_t pmk[ESP_NOW_KEY_LEN];
    for ( int i = 0; i < ESP_NOW_KEY_LEN; i++ )
    {
        pmk[i] = config.key[i] ^ HXRC_VERSION; 
    }

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

    if ( esp_now_set_kok( pmk, ESP_NOW_KEY_LEN ) != ESP_OK )
    {
        Serial.println("HXESPNOWRC: Error: Failed to set pmk");
        return false;
    }

    if ( esp_now_add_peer(config.peer_mac, ESP_NOW_ROLE_COMBO, config.wifi_channel, config.key, ESP_NOW_KEY_LEN) != ESP_OK )
    //if ( esp_now_add_peer(config.peer_mac, ESP_NOW_ROLE_COMBO, config.wifi_channel, NULL, 0) != ESP_OK )
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
    if ( esp_wifi_set_ps(WIFI_PS_NONE) != ESP_OK )
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

    if ( esp_now_set_pmk(pmk) != ESP_OK )
    {
        Serial.println("HXESPNOWRC: Error: Failed to set pmk");
        return false;
    }

    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, config.peer_mac, 6);
    peerInfo.channel = config.wifi_channel;
    memcpy(peerInfo.lmk, config.key, ESP_NOW_KEY_LEN);
    peerInfo.encrypt = true;
    peerInfo.ifidx = WIFI_IF_STA;

    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("HXRC: Error: Failed to add peer");
        return false;
    }

#endif



    return true;
}

//=====================================================================
//=====================================================================
void HXRC_crc16_update( uint16_t* crc, uint8_t a )
{
    (*crc) ^= a;
    for ( uint8_t i = 0; i < 8; ++i )
    {
        if ( (*crc) & 1 )
        {
            (*crc) = ( (*crc) >> 1 ) ^ 0xA001;
        }
        else
        {
            (*crc) = ( (*crc) >> 1 );
        }
    }       
}

//=====================================================================
//=====================================================================
void HXRC_crc16_update_buffer( uint16_t* crc, uint8_t* ptr, uint16_t len )
{
    while ( len-- > 0 ) HXRC_crc16_update( crc, *ptr++);
}

