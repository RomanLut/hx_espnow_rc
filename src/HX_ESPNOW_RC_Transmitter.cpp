#include "HX_ESPNOW_RC_Transmitter.h"
#include "HX_ESPNOW_RC_Common.h"


/*
#if defined(ESP8266)
#pragma message "ESP8266 stuff happening!"
#elif defined(ESP32)
#pragma message "ESP32 stuff happening!"
#else
#error "This ain't a ESP8266 or ESP32, dumbo!"
#endif
*/


#if defined(ESP8266)

#include <ESP8266WiFi.h>
#include <espnow.h>

#define ESP_OK 0
#define esp_err_t int
//#define esp_now_send_status_t uint8_t
#define ESP_NOW_SEND_SUCCESS 0

#elif defined(ESP32)

#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

#endif

static HXRCTransmitterStats transmitterStats;
static HXRCReceiverStats receiverStats;
static HXRCSenderStateEnum senderState;

static HXRCPayloadTransmitter outgoingData;

// REPLACE WITH THE MAC Address of your receiver 
//quad {0x30, 0xAE, 0xA4, 0x99, 0x28, 0xB4} - STA
//quad {0x30, 0xAE, 0xA4, 0x99, 0x28, 0xB5} - AP
uint8_t broadcastAddress[] = {0x30, 0xAE, 0xA4, 0x99, 0x28, 0xB5};


//doit esp32dev v1 7C:9E:BD:F5:0D:6D sta
//uint8_t broadcastAddress[] = {0x7c, 0x9e, 0xbd, 0xf5, 0x0d, 0x6d};

//esp32cam sta F0:08:D1:CC:B3:11
//uint8_t broadcastAddress[] = {0xf0, 0x08, 0xd1, 0xcc, 0xb3, 0x11};

// Callback when data is sent
#if defined(ESP8266)
static void OnDataSent(uint8_t *mac_addr, uint8_t status)
#elif defined(ESP32)
static void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
#endif
{
    bool success = status == ESP_NOW_SEND_SUCCESS;

    if( success )
    {
        transmitterStats.onPacketSendSuccess( outgoingData.length );
        //todo: free telemetry buffer
    }
    else    
    {
        transmitterStats.onPacketSendError();
    }

    senderState = HXRCSS_READY_TO_SEND;
}

// Callback when data is received
#if defined(ESP8266)
static void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len)
#elif defined (ESP32)
static void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
#endif
{
    const HXRCPayloadReceiver* pPayload = ( const HXRCPayloadReceiver*) incomingData;

    if ( len >= HXRC_RECEIVER_PAYLOAD_SIZE_BASE )
    {
        receiverStats.onPacketReceived( pPayload->sequenceId, pPayload->rssi, pPayload->length );
        //todo:copy telemetry data pPayloadTelemetry.length               
    }
    else
    {
        //ignore too short payload
    }
}


void HXRCInit( int wifi_channel )
{
    transmitterStats.reset();
    receiverStats.reset();

    outgoingData.sequenceId = 0;
    for ( int i =0; i < HXRC_CHANNELS; i++ )
    {
        HXRCSetChannelValue( i, 1000);
    }
    senderState = HXRCSS_READY_TO_SEND;

#if defined(ESP8266)
    
    WiFi.mode(WIFI_AP);

#elif defined(ESP32)
    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_AP);

    if ( esp_wifi_set_protocol (WIFI_IF_AP, WIFI_PROTOCOL_11B) != ESP_OK)
    {
        Serial.println("HXRC: Error: Failed to enable LR mode");
        return;
    }

    WiFi.softAP("hxrct", NULL, wifi_channel);

#endif

    WiFi.softAP("hxrct", NULL, wifi_channel);

    Serial.println("HXESPNOWRC: Info: Board MAC address:");
    //Serial.println(WiFi.macAddress());
    Serial.println(WiFi.softAPmacAddress());

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK)
    {
        Serial.println("HXESPNOWRC: Error: Error initializing ESP-NOW");
        return;
    }

#if defined(ESP8266)

    if ( esp_now_set_self_role(ESP_NOW_ROLE_COMBO) != ESP_OK )
    {
        Serial.println("HXRC: Error: Failed to set role");
        return;
    }

    esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, wifi_channel, NULL, 0);

#elif defined(ESP32)

    // Register peer
    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = wifi_channel;
    peerInfo.encrypt = false;
    peerInfo.ifidx = WIFI_IF_AP;

    // Add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("HXRC: Error: Failed to add peer");
        return;
    }

#endif


    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    esp_now_register_send_cb(OnDataSent);

    // Register for a callback function that will be called when data is received
    esp_now_register_recv_cb(OnDataRecv);

    //Serial.print("Packetsize: ");
    //Serial.println(sizeof(HXRCPayloadTransmitter));
    
}

void HXRCLoop()
{
    unsigned long t = millis();
    unsigned long deltaT = t - transmitterStats.lastSendTimeMs;

    if (senderState == HXRCSS_READY_TO_SEND )
    {
        int count = deltaT / PACKET_SEND_PERIOD_MS;

        if ( count > 1)
        {
            transmitterStats.onPacketSendMiss( count - 1 );           
            outgoingData.sequenceId += count - 1;
        }

        outgoingData.length = 0;
        //if (hasTelemetry())
        {
            outgoingData.length = HXRC_TRANSMITTER_TELEMETRY_SIZE_MAX;
        }

        transmitterStats.onPacketSend( t );

        outgoingData.sequenceId++;
        esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &outgoingData, HXRC_TRANSMITTER_PAYLOAD_SIZE_BASE + outgoingData.length);
        if (result == ESP_OK) 
        {
            senderState = HXRCSS_WAITING_CONFIRMATION;
        }
        else 
        {
            //Serial.println("immediate send error");
            transmitterStats.onPacketSendError();
        }            
    }

    transmitterStats.update();
    receiverStats.update();

}

void HXRCSetChannelValue(uint8_t index, uint16_t data)
{
    HXRCSetChannelValueInt( outgoingData.channels, index, data);
}

HXRCTransmitterStats& HXRCGetTransmitterStats() 
{
    return transmitterStats;
}

HXRCReceiverStats& HXRCGetReceiverStats() 
{
    return receiverStats;
}

