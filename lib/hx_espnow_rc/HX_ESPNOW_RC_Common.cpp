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
//https://esp32.com/viewtopic.php?t=13889
#if defined(ESP32)
HXRCPromiscuousCapture capture;

static void ICACHE_RAM_ATTR sniffer_callback(void *buf, wifi_promiscuous_pkt_type_t type)
{
    if  (type != WIFI_PKT_MGMT) return;

    static const uint8_t ACTION_SUBTYPE = 0xd0;

    const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buf;
    const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
    const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;

    if ( 
        (ACTION_SUBTYPE == (hdr->frame_ctrl & 0xFF) ) &&
        (memcmp( hdr->addr2, capture.peerMac, 6 ) == 0 )  //mac is first 6 digits
    )
    {
        capture.rssi = ppkt->rx_ctrl.rssi;
        capture.noiseFloor = ppkt->rx_ctrl.noise_floor;
        capture.rate = ppkt->rx_ctrl.rate;
        capture.packetsCount++;
    }
}
#endif

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
    //https://www.esp32.com/viewtopic.php?t=12772
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

    system_phy_set_max_tpw(82);

#elif defined(ESP32)

    WiFi.mode(WIFI_STA);

/*
    Can't set rate without disabling AMPDU, disabling is impossible with Arduino SDK :(
    "If you want to use esp_wifi_internal_set_fix_rate, please disable WiFi AMPDU TX by:
     make menuconfig => components => Wi-Fi => Disable TX AMPDU."

    if( config.LRMode)
    {
        if ( esp_wifi_internal_set_fix_rate(ESP_IF_WIFI_STA, true, WIFI_PHY_RATE_LORA_250K) != true )  //WIFI_PHY_RATE_1M_L
        {
            Serial.println("HXRC: Error: Failed to set rate");
            //return false;
        }
    }
*/
    esp_wifi_set_promiscuous(true); //promiscous mode is required to set channel on older SDK
    if ( esp_wifi_set_channel( config.wifi_channel, WIFI_SECOND_CHAN_NONE) != ESP_OK )
    {
        Serial.println("HXRC: Error: Failed to set channel");
        return false;
    }
    esp_wifi_set_promiscuous(false); 

    //WIFI_PROTOCOL_11B - device will be able to send and receive WIFI_PROTOCOL_11B packets
    //WIFI_PROTOCOL_LR | WIFI_PROTOCOL_11B - device will be able to receive WIFI_PROTOCOL_LR and WIFI_PROTOCOL_11B,
    //it will send either WIFI_PROTOCOL_LR or WIFI_PROTOCOL_11B depending on supported type on peer
    //WIFI_PROTOCOL_LR - device will be able to receive WIFI_PROTOCOL_LR and WIFI_PROTOCOL_11B, will send WIFI_PROTOCOL_LR only.
    //If  WIFI_PROTOCOL_LR connects to (WIFI_PROTOCOL_LR | WIFI_PROTOCOL_11B), packets in one direction are WIFI_PROTOCOL_LR,
    //packtes in other direction are  WIFI_PROTOCOL_11B
    //So to force full LR communication, both peers should use exclusively WIFI_PROTOCOL_LR mode
    if ( esp_wifi_set_protocol (WIFI_IF_STA, config.LRMode ? WIFI_PROTOCOL_LR : ( /*WIFI_PROTOCOL_LR |*/ WIFI_PROTOCOL_11B ) ) != ESP_OK)
    {
        Serial.println("HXRC: Error: Failed to switch LR mode");
        return false;
    }

    //set bandwidth to 20Mhz to decrease noise floor by 3dbm
    //review: does it have any effect?
    if ( esp_wifi_set_bandwidth(WIFI_IF_STA, WIFI_BW_HT20) != ESP_OK )
    {
        Serial.println("HXRC: Error: Failed to set bandwidth");
        return false;
    } 

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

    esp_wifi_set_promiscuous_rx_cb(&sniffer_callback);
    esp_wifi_set_promiscuous(true); 

    if (  esp_wifi_set_max_tx_power(84) != ESP_OK )
  {
    Serial.println("An error occurred while setting TX power");
  }
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

//=====================================================================
//=====================================================================
void HXRCPrintMac( const uint8_t* mac ) 
{
    for ( int i = 0; i < 6; i++ ) 
    {
        if ( mac[i]<16 ) HXRCLOG.print("0");
        HXRCLOG.print(mac[i],HEX);
        if ( i < 5 ) HXRCLOG.print(":");
    }
}