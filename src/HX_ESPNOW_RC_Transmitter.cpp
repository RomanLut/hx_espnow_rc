#include "HX_ESPNOW_RC_Transmitter.h"
#include "HX_ESPNOW_RC_Common.h"

#include <esp_now.h>
#include <WiFi.h>

static HXRCTransmitterStats transmitterStats;
static HXRCReceiverStats receiverStats;
static HXRCSenderStateEnum senderState;
static bool lastSentPacketIsTelemetry;
static uint16_t outgoingSequenceId;

static HXRCPayloadChannels outgoingRCData;
static HXRCPayloadTelemetry outgoingTelemetry;

// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0x30, 0xAE, 0xA4, 0x99, 0x28, 0xB5};
//quad {0x30, 0xAE, 0xA4, 0x99, 0x28, 0xB4} - STA
//quad {0x30, 0xAE, 0xA4, 0x99, 0x28, 0xB5} - AP

// Callback when data is sent
static void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    bool success = status == ESP_NOW_SEND_SUCCESS;

    if( success )
    {
        transmitterStats.onPacketSendSuccess();
        if ( lastSentPacketIsTelemetry )
        {
            transmitterStats.telemetryBytesSentTotal += outgoingTelemetry.length;
        }
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
    //memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
    //Serial.print("Bytes received: ");
    //Serial.println(len);

    const HXRCPayload* pPayload = ( const HXRCPayload*) incomingData;

    if ( len >= HXRCPayloadSize_MIN )
    {
        receiverStats.onPacketReceived( pPayload );
        if ( pPayload->packetId == HXRCPKID_TELEMETRY )
        {
            const HXRCPayloadTelemetry* pPayloadTelemetry = (const HXRCPayloadTelemetry*)pPayload;
            //copy telemetry data pPayloadTelemetry.length               
        }
        else
        {
            //unknown packet id?
        }
    }
    else
    {
        //ignore too short payload
    }
}


void HXRCInit( int wifi_channel )
{
    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_AP);

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

    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    esp_now_register_send_cb(OnDataSent);

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
    // Register for a callback function that will be called when data is received
    esp_now_register_recv_cb(OnDataRecv);

    for ( int i =0; i < HXRC_CHANNELS; i++ )
    {
        HXRCSetChannelValue( i, 1000);
    }

    outgoingRCData.packetId = HXRCPKID_RCDATA;
    outgoingTelemetry.packetId = HXRCPKID_TELEMETRY;
    outgoingSequenceId = 0;

    senderState = HXRCSS_READY_TO_SEND;

    //Serial.print("Packetsize: ");
    //Serial.println(sizeof(HXRCPayloadChannels));
    
}

void HXRCLoop()
{
    unsigned long t = millis();

    unsigned long deltaT = t - transmitterStats.lastSendTimeMs;

    if (senderState == HXRCSS_READY_TO_SEND )
    {
        //sanitize long deltas
        if ( deltaT > PACKET_SEND_PERIOD_MS * 32) 
        {
            deltaT = PACKET_SEND_PERIOD_MS * 32;
            transmitterStats.lastSendTimeMs = t - deltaT;
        }

        while ( deltaT >= PACKET_SEND_PERIOD_MS*2 )
        {
            //packets were not sent in time. Concider they are lost.                
            transmitterStats.onPacketSendMiss();
            transmitterStats.lastSendTimeMs += PACKET_SEND_PERIOD_MS;
            deltaT -= PACKET_SEND_PERIOD_MS;
        }

        if ( deltaT >= PACKET_SEND_PERIOD_MS )
        {
            outgoingSequenceId++; 
            outgoingRCData.sequenceId = outgoingSequenceId;

            transmitterStats.packetsSentTotal++;
            transmitterStats.lastSendTimeMs += PACKET_SEND_PERIOD_MS;

            lastSentPacketIsTelemetry = false;
            esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &outgoingRCData, sizeof(outgoingRCData));
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
        else
        {
            //if ( outgoingTelemetryBuggerHasData)
            {
                outgoingSequenceId++; 
                outgoingTelemetry.sequenceId = outgoingSequenceId;
                outgoingTelemetry.rssi = -1; //do not send rssi to uav

                outgoingTelemetry.length = HXRCTelemetryDataSizeMAX;

                transmitterStats.packetsSentTotal++;

                lastSentPacketIsTelemetry = true;
                esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &outgoingTelemetry, sizeof(HXRCTelemetryHeaderSize + outgoingTelemetry.length));
                if (result == ESP_OK) 
                {
                    senderState = HXRCSS_WAITING_CONFIRMATION;
                }
                else 
                {
                    //Serial.println("immediate send error");
                    transmitterStats.packetsSentError++;
                }            
            }
        }

    }

}

void HXRCSetChannelValue(uint8_t index, uint16_t data)
{
    HXRCSetChannelValueInt( outgoingRCData, index, data);
}

HXRCTransmitterStats& HXRCGetTransmitterStats() 
{
    return transmitterStats;
}

HXRCReceiverStats& HXRCGetReceiverStats() 
{
    return receiverStats;
}

