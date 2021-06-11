#include "HX_ESPNOW_RC_Common.h"
#include "HX_ESPNOW_RC_Receiver.h"

#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

// REPLACE WITH THE MAC Address of your receiver 
//ttgo-display 24:62:AB:CA:AA:DD - AP
//uint8_t broadcastAddress[] = {0x24, 0x62, 0xAB, 0xCA, 0xAA, 0xDD};


//quad {0x30, 0xAE, 0xA4, 0x99, 0x28, 0xB5} - AP
uint8_t broadcastAddress[] = {0x30, 0xAE, 0xA4, 0x99, 0x28, 0xB5};


static HXRCTransmitterStats transmitterStats;
static HXRCReceiverStats receiverStats;
static HXRCSenderStateEnum senderState;

static HXRCChannels receivedChannels;

static HXRCPayloadReceiver outgoingTelemetry;

// Callback when data is sent
static void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    bool success = status == ESP_NOW_SEND_SUCCESS;

    if( success )
    {
        transmitterStats.onPacketSendSuccess(outgoingTelemetry.length );
        //todo: free telemetry buffer
    }
    else    
    {
        transmitterStats.onPacketSendError();
    }

    senderState = HXRCSS_READY_TO_SEND;
}

// Callback when data is received
static void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    if ( len >= HXRC_TRANSMITTER_PAYLOAD_SIZE_BASE )
    {
        const HXRCPayloadTransmitter* pPayload = (const HXRCPayloadTransmitter*) incomingData;
        memcpy(&receivedChannels, &pPayload->channels, sizeof(receivedChannels));

        //todo:process telemetry
        receiverStats.onPacketReceived( pPayload->sequenceId, -1, pPayload->length );

        //todo: copy incoming telemetry
    }
    else
    {
        //ignore unknown packet, too short
        Serial.print("Unknown packet length:");
        Serial.println(len);
    }
}

//channel - channel where AP was is initialized
//WiFi.softAP("quad", NULL, 13);
void HXRCInit( int channel )
{
    outgoingTelemetry.sequenceId = 0;
    for ( int i = 0; i < HXRC_CHANNELS; i++ )
    {
        HXRCSetChannelValueInt( receivedChannels, i, 1000 );
    }

    senderState = HXRCSS_READY_TO_SEND;
  
    Serial.println("HXESPNOWRC: Info: Board MAC address:");
    //Serial.println(WiFi.macAddress());
    Serial.println(WiFi.softAPmacAddress());

/*
    if ( esp_wifi_set_protocol (WIFI_IF_AP, WIFI_PROTOCOL_LR) != ESP_OK)
    {
        Serial.println("HXRC: Error: Failed to enable LR mode");
        return;
    }
*/
    // Init ESP-NOW
    if (esp_now_init() != ESP_OK)
    {
        Serial.println("HXESPNOWRC: Error: Error initializing ESP-NOW");
        return;
    }

    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    esp_now_register_send_cb(OnDataSent);

    // Register peer
    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = channel;
    peerInfo.encrypt = false;
    peerInfo.ifidx = WIFI_IF_AP;

    // Add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("HXRC: Error: Failed to add peer");
        return;
    }
    // Register for a callback function that will be called when data is received
    esp_now_register_recv_cb(OnDataRecv);
}

void HXRCLoop()
{
    unsigned long t = millis();
    unsigned long deltaT = t - transmitterStats.lastSendTimeMs;

    if ( senderState == HXRCSS_READY_TO_SEND )
    {
        outgoingTelemetry.sequenceId++;
        outgoingTelemetry.rssi = receiverStats.getRSSI();
        
        //TODO: send any telemetry data or empty packet
        outgoingTelemetry.length = HXRC_RECEIVER_TELEMETRY_SIZE_MAX;  

        transmitterStats.onPacketSend( t );
        esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &outgoingTelemetry, HXRC_RECEIVER_PAYLOAD_SIZE_BASE + outgoingTelemetry.length );
        if (result == ESP_OK) 
        {
            senderState = HXRCSS_WAITING_CONFIRMATION;
        }
        else 
        {
            transmitterStats.onPacketSendError();
        }            
    }

    transmitterStats.update();
    receiverStats.update();
}

uint16_t HXRCGetChannelValue(uint8_t index )
{
    return HXRCGetChannelValueInt( receivedChannels, index );
}

HXRCReceiverStats& HXRCGetReceiverStats() 
{
    return receiverStats;
}

HXRCTransmitterStats& HXRCGetTransmitterStats() 
{
    return transmitterStats;
}

