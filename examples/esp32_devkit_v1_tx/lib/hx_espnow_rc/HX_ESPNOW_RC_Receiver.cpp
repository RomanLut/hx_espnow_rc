#include "HX_ESPNOW_RC_Receiver.h"

HXRCReceiver* HXRCReceiver::pInstance;

#if defined(ESP8266)
void HXRCReceiver::OnDataSentStatic(uint8_t *mac_addr, uint8_t status) {HXRCReceiver::pInstance->OnDataSent( mac_addr, status );};
void HXRCReceiver::OnDataRecvStatic(uint8_t *mac, uint8_t *incomingData, uint8_t len) {HXRCReceiver::pInstance->OnDataRecv( mac, incomingData, len);};
#elif defined (ESP32)
static void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
static void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);
#endif

//=====================================================================
//=====================================================================
// Callback when data is sent
#if defined(ESP8266)
void HXRCReceiver::OnDataSent(uint8_t *mac_addr, uint8_t status)
#elif defined (ESP32)
void HXRCReceiver::OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
#endif
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

//=====================================================================
//=====================================================================
// Callback when data is received
#if defined(ESP8266)
void HXRCReceiver::OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len)
#elif defined (ESP32)
void HXRCReceiver::OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
#endif
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

//=====================================================================
//=====================================================================
//channel - channel where AP was is initialized
//WiFi.softAP("quad", NULL, 13);
void HXRCReceiver::init( HXRCConfig config )
{
    this->config = config;

    if ( config.ledPin != -1 )
    {
        pinMode(config.ledPin,OUTPUT);
        digitalWrite(this->config.ledPin, this->config.ledPinInverted ? HIGH : LOW );
    }

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
    esp_now_register_send_cb(OnDataSentStatic);

#if defined(ESP8266)

    if ( esp_now_set_self_role(ESP_NOW_ROLE_COMBO) != ESP_OK )
    {
        Serial.println("HXRC: Error: Failed to set role");
        return;
    }

    esp_now_add_peer(this->config.peer_mac, ESP_NOW_ROLE_SLAVE, this->config.wifi_channel, NULL, 0);

#elif defined(ESP32)

    // Register peer
    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, this->config.peer_mac, 6);
    peerInfo.channel = this->config.wifi_channel;
    peerInfo.encrypt = false;
    peerInfo.ifidx = WIFI_IF_AP;

    // Add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("HXRC: Error: Failed to add peer");
        return;
    }
#endif

    // Register for a callback function that will be called when data is received
    esp_now_register_recv_cb(OnDataRecvStatic);
}

//=====================================================================
//=====================================================================
void HXRCReceiver::loop()
{
    unsigned long t = millis();

    if ( senderState == HXRCSS_READY_TO_SEND )
    {
        outgoingTelemetry.sequenceId++;
        outgoingTelemetry.rssi = receiverStats.getRSSI();
        
        //TODO: send any telemetry data or empty packet
        outgoingTelemetry.length = HXRC_RECEIVER_TELEMETRY_SIZE_MAX;  

        transmitterStats.onPacketSend( t );
        esp_err_t result = esp_now_send(this->config.peer_mac, (uint8_t *) &outgoingTelemetry, HXRC_RECEIVER_PAYLOAD_SIZE_BASE + outgoingTelemetry.length );
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

    updateLed();
}

//=====================================================================
//=====================================================================
uint16_t HXRCReceiver::getChannelValue(uint8_t index )
{
    return HXRCGetChannelValueInt( receivedChannels, index );
}

//=====================================================================
//=====================================================================
HXRCReceiverStats& HXRCReceiver::getReceiverStats() 
{
    return receiverStats;
}

//=====================================================================
//=====================================================================
HXRCTransmitterStats& HXRCReceiver::getTransmitterStats() 
{
    return transmitterStats;
}

//=====================================================================
//=====================================================================
void HXRCReceiver::updateLed()
{
    if ( config.ledPin == -1) return;

    if ( getTransmitterStats().isFailsafe())
    {
        digitalWrite(this->config.ledPin, this->config.ledPinInverted ? HIGH : LOW );
    }
    else
    {
        digitalWrite(config.ledPin,(millis() & 32) ? HIGH : LOW );
    }
}

