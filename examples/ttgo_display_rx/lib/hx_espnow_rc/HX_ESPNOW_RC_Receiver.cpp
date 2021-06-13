#include "HX_ESPNOW_RC_Receiver.h"

HXRCReceiver* HXRCReceiver::pInstance;

#if defined(ESP8266)
void HXRCReceiver::OnDataSentStatic(uint8_t *mac_addr, uint8_t status) {HXRCReceiver::pInstance->OnDataSent( mac_addr, status );};
void HXRCReceiver::OnDataRecvStatic(uint8_t *mac, uint8_t *incomingData, uint8_t len) {HXRCReceiver::pInstance->OnDataRecv( mac, incomingData, len);};
#elif defined (ESP32)
void HXRCReceiver::OnDataSentStatic(const uint8_t *mac_addr, esp_now_send_status_t status) {HXRCReceiver::pInstance->OnDataSent( mac_addr, status );};
void HXRCReceiver::OnDataRecvStatic(const uint8_t *mac, const uint8_t *incomingData, int len) {HXRCReceiver::pInstance->OnDataRecv( mac, incomingData, len);};
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
    if ( len >= HXRC_MASTER_PAYLOAD_SIZE_BASE )
    {
        const HXRCPayloadMaster* pPayload = (const HXRCPayloadMaster*) incomingData;
        memcpy(&receivedChannels, &pPayload->channels, sizeof(receivedChannels));

        //todo:process telemetry
        receiverStats.onPacketReceived( pPayload->sequenceId, -2, pPayload->length );

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
bool HXRCReceiver::init( HXRCConfig config )
{
    this->config = config;

    transmitterStats.reset();
    receiverStats.reset();

    HXRCInitLedPin(config);

    outgoingTelemetry.sequenceId = 0;
    for ( int i = 0; i < HXRC_CHANNELS; i++ )
    {
        HXRCSetChannelValueInt( receivedChannels, i, 1000 );
    }

    senderState = HXRCSS_READY_TO_SEND;

    if ( !HXRCInitEspNow( config, "hxrcr"))
    {
        return false;
    }

    esp_now_register_recv_cb(OnDataRecvStatic);
    esp_now_register_send_cb(OnDataSentStatic);

    return true;
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
        outgoingTelemetry.length = HXRC_SLAVE_TELEMETRY_SIZE_MAX;  

        transmitterStats.onPacketSend( t );
        esp_err_t result = esp_now_send(this->config.peer_mac, (uint8_t *) &outgoingTelemetry, HXRC_SLAVE_PAYLOAD_SIZE_BASE + outgoingTelemetry.length );
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

