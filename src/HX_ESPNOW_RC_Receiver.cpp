#include "HX_ESPNOW_RC_Common.h"
#include "HX_ESPNOW_RC_Receiver.h"

#include <esp_now.h>
#include <WiFi.h>

// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0x24, 0x62, 0xAB, 0xCA, 0xAA, 0xDD};
//ttgo-display 24:62:AB:CA:AA:DC - STA



static HXRCTransmitterStats transmitterStats;
static HXRCReceiverStats receiverStats;
static HXRCSenderStateEnum senderState;

static HXRCPayloadChannels receivedChannels;

static HXRCPayloadTelemetry outgoingTelemetry;
static HXRCPayloadTelemetry incomingTelemetry;  //TODO

// Callback when data is sent
static void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    bool success = status == ESP_NOW_SEND_SUCCESS;

    if( success )
    {
        transmitterStats.onPacketSendSuccess();
        transmitterStats.lastTelemetryBytesSentTotal += outgoingTelemetry.length;
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
    if ( len >=HXRCPayloadSize_MIN )
    {
        const HXRCPayload* pPayload = (const HXRCPayload*) incomingData;
        if (pPayload->packetId == HXRCPKID_RCDATA)
        {
            memcpy(&receivedChannels, incomingData, sizeof(receivedChannels));
        }
        else if (pPayload->packetId == HXRCPKID_TELEMETRY)
        {
            memcpy(&incomingTelemetry, incomingData, sizeof(incomingTelemetry));
        }
        receiverStats.onPacketReceived( pPayload );
    }
    else
    {
        //ignore unknown packet, too short
    }
}

//channel - channel where AP was is initialized
//WiFi.softAP("quad", NULL, 13);
void HXRCInit( int channel )
{
    Serial.println("HXESPNOWRC: Info: Board MAC address:");
    //Serial.println(WiFi.macAddress());
    Serial.println(WiFi.softAPmacAddress());

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

    for ( int i = 0; i < HXRC_CHANNELS; i++ )
    {
        HXRCSetChannelValueInt( receivedChannels, i, 1000 );
    }

    senderState = HXRCSS_READY_TO_SEND;
    
    outgoingTelemetry.packetId = HXRCPKID_TELEMETRY;
    outgoingTelemetry.sequenceId = 0;
}

void HXRCLoop()
{
    unsigned long t = millis();

    unsigned long deltaT = t - transmitterStats.lastSendTimeMs;

    if ( senderState == HXRCSS_READY_TO_SEND )
    {
        //if (hastelemetry)
        {
            outgoingTelemetry.sequenceId++;

            outgoingTelemetry.rssi = receiverStats.getRSSI();
            
            //TODO: send any telemetry data
            outgoingTelemetry.length = HXRCTelemetryDataSizeMAX;  

            transmitterStats.packetsSentTotal++;

            esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &outgoingTelemetry, HXRCTelemetryHeaderSize + outgoingTelemetry.length );
            if (result == ESP_OK) 
            {
                senderState = HXRCSS_WAITING_CONFIRMATION;
            }
            else 
            {
                transmitterStats.onPacketSendError();
            }            
        }
    }
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

