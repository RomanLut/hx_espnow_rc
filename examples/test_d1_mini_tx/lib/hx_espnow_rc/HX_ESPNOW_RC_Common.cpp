#include "HX_ESPNOW_RC_Common.h"

//=====================================================================
//=====================================================================
uint16_t HXRCGetChannelValueInt(const HXRCChannels& channels, uint8_t index )
{
    switch( index )
    {
        case 0:
            return channels.Ch1 + 1000;
        case 1:
            return channels.Ch2 + 1000;
        case 2:
            return channels.Ch3 + 1000;
        case 3:
            return channels.Ch4 + 1000;
        case 4:
            return channels.Ch5 + 1000;
        case 5:
            return channels.Ch6 + 1000;
        case 6:
            return channels.Ch7 + 1000;
        case 7:
            return channels.Ch8 + 1000;
        case 8:
            return channels.Ch9 + 1000;
        case 9:
            return channels.Ch10 + 1000;
        case 10:
            return channels.Ch11 + 1000;
        case 11:
            return channels.Ch12 + 1000;
        case 12:
            return channels.Ch13 + 1000;
        case 13:
            return channels.Ch14 + 1000;
        case 14:
            return channels.Ch15 + 1000;
        case 15:
            return channels.Ch16 + 1000;
        default:
            return 1000;
    }
}

//=====================================================================
//=====================================================================
void HXRCSetChannelValueInt(HXRCChannels& channels, uint8_t index, uint16_t data)
{
    data -= 1000;
    switch( index )
    {
        case 0:
            channels.Ch1 = data;
            break;
        case 1:
            channels.Ch2 = data;
            break;
        case 2:
            channels.Ch3 = data;
            break;
        case 3:
            channels.Ch4 = data;
            break;
        case 4:
            channels.Ch5 = data;
            break;
        case 5:
            channels.Ch6 = data;
            break;
        case 6:
            channels.Ch7 = data;
            break;
        case 7:
            channels.Ch8 = data;
            break;
        case 8:
            channels.Ch9 = data;
            break;
        case 9:
            channels.Ch10 = data;
            break;
        case 10:
            channels.Ch11 = data;
            break;
        case 11:
            channels.Ch12 = data;
            break;
        case 12:
            channels.Ch13 = data;
            break;
        case 13:
            channels.Ch14 = data;
            break;
        case 14:
            channels.Ch15 = data;
            break;
        case 15:
            channels.Ch16 = data;
            break;
    }
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
bool HXRCInitEspNow( HXRCConfig& config, const char* ssid )
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

    if ( esp_now_set_self_role(ESP_NOW_ROLE_COMBO) != ESP_OK )
    {
        Serial.println("HXRC: Error: Failed to set role");
        return false;
    }

    if ( esp_now_add_peer(config.peer_mac, ESP_NOW_ROLE_COMBO, config.wifi_channel, NULL, 0) != ESP_OK )
    {
        Serial.println("HXRC: Error: Failed to add peer");
        return false;
    }

#elif defined(ESP32)

    WiFi.mode(WIFI_STA);

    //esp_wifi_set_promiscuous(true); 
    if ( esp_wifi_set_channel( config.wifi_channel, WIFI_SECOND_CHAN_NONE) != ESP_OK )
    {
        Serial.println("HXRC: Error: Failed to set channel");
        return false;
    }
    //esp_wifi_set_promiscuous(false); 

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

    if (esp_now_init() != ESP_OK)
    {
        Serial.println("HXESPNOWRC: Error: Error initializing ESP-NOW");
        return false;
    }

    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, config.peer_mac, 6);
    peerInfo.channel = config.wifi_channel;
    peerInfo.encrypt = false;
    peerInfo.ifidx = WIFI_IF_STA;

    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("HXRC: Error: Failed to add peer");
        return false;
    }
#endif

    return true;
}
