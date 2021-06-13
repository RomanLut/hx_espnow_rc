#include "HX_ESPNOW_RC_Transmitter.h"

HXRCTransmitter* HXRCTransmitter::pInstance;

#if defined(ESP8266)
void HXRCTransmitter::OnDataSentStatic(uint8_t *mac_addr, uint8_t status) {HXRCTransmitter::pInstance->OnDataSent( mac_addr, status );};
void HXRCTransmitter::OnDataRecvStatic(uint8_t *mac, uint8_t *incomingData, uint8_t len) {HXRCTransmitter::pInstance->OnDataRecv( mac, incomingData, len);};
#elif defined (ESP32)
void HXRCTransmitter::OnDataSentStatic(const uint8_t *mac_addr, esp_now_send_status_t status) {HXRCTransmitter::pInstance->OnDataSent( mac_addr, status );};
void HXRCTransmitter::OnDataRecvStatic(const uint8_t *mac, const uint8_t *incomingData, int len) {HXRCTransmitter::pInstance->OnDataRecv( mac, incomingData, len);};
#endif

//=====================================================================
//=====================================================================
// Callback when data is sent
#if defined(ESP8266)
void HXRCTransmitter::OnDataSent(uint8_t *mac_addr, uint8_t status)
#elif defined(ESP32)
void HXRCTransmitter::OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
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

//=====================================================================
//=====================================================================
// Callback when data is received
#if defined(ESP8266)
void HXRCTransmitter::OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len)
#elif defined (ESP32)
void HXRCTransmitter::OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
#endif
{
    const HXRCPayloadSlave* pPayload = ( const HXRCPayloadSlave*) incomingData;

    if ( ( len >= HXRC_SLAVE_PAYLOAD_SIZE_BASE ) &&  ( len == HXRC_SLAVE_PAYLOAD_SIZE_BASE + pPayload->length ) )
    {
        receiverStats.onPacketReceived( pPayload->sequenceId, pPayload->rssi, pPayload->length );

        uint16_t lenToWrite = pPayload->length;
        uint16_t freeCount = this->incomingTelemetryBufffer.getFreeCount();

        if ( lenToWrite > freeCount )
        {
            lenToWrite = freeCount;  
            receiverStats.onTelemetryOverflow();
        }
        this->incomingTelemetryBufffer.insertBuffer( pPayload->data, lenToWrite );
        if ( this->incomingTelemetryCallback )
        {
            this->incomingTelemetryCallback( this->incomingTelemetryCallbackParm, *this );
        }
    }
    else
    {
        //ignore too short payload
        Serial.println("HXRC: Error: invalid payload length");
    }
}

//=====================================================================
//=====================================================================
bool HXRCTransmitter::init( HXRCConfig config )
{
    this->config = config;

    transmitterStats.reset();
    receiverStats.reset();

    HXRCInitLedPin(config);

    outgoingData.sequenceId = 0;
    for ( int i =0; i < HXRC_CHANNELS; i++ )
    {
        setChannelValue( i, 1000);
    }
    senderState = HXRCSS_READY_TO_SEND;

    if ( !HXRCInitEspNow( config, "hxrct" ) )
    {
        return false;
    }

    esp_now_register_send_cb(OnDataSentStatic);
    esp_now_register_recv_cb(OnDataRecvStatic);

    return true;
}

//=====================================================================
//=====================================================================
void HXRCTransmitter::loop()
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
            outgoingData.length = HXRC_MASTER_TELEMETRY_SIZE_MAX;
        }

        transmitterStats.onPacketSend( t );

        outgoingData.sequenceId++;
        esp_err_t result = esp_now_send( this->config.peer_mac, (uint8_t *) &outgoingData, HXRC_MASTER_PAYLOAD_SIZE_BASE + outgoingData.length);
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

    updateLed();
}

//=====================================================================
//=====================================================================
void HXRCTransmitter::setChannelValue(uint8_t index, uint16_t data)
{
    HXRCSetChannelValueInt( outgoingData.channels, index, data);
}

//=====================================================================
//=====================================================================
HXRCTransmitterStats& HXRCTransmitter::getTransmitterStats() 
{
    return transmitterStats;
}

//=====================================================================
//=====================================================================
HXRCReceiverStats& HXRCTransmitter::getReceiverStats() 
{
    return receiverStats;
}

//=====================================================================
//=====================================================================
void HXRCTransmitter::updateLed()
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

//=====================================================================
//=====================================================================
void HXRCTransmitter::addIncomingTelemetryCallback( void (*callback)(void* parm, HXRCTransmitter& transmitter), void* parm)
{
    this->incomingTelemetryCallback = callback;
    this->incomingTelemetryCallbackParm = parm;
}

//=====================================================================
//=====================================================================
void HXRCTransmitter::addOutgoingTelemetryCallback( void (*callback)(void* parm, HXRCTransmitter& transmitter), void* parm)
{
    this->outgoingTelemetryCallback = callback;
    this->outgoingTelemetryCallbackParm = parm;
}

//=====================================================================
//=====================================================================
HXRCRingBufer<HXRC_TELEMETRY_BUFFER_SIZE>& HXRCTransmitter::getIncomingTelemetryBufffer()
{
    return this->incomingTelemetryBufffer;
}

//=====================================================================
//=====================================================================
HXRCRingBufer<HXRC_TELEMETRY_BUFFER_SIZE>& HXRCTransmitter::getOutgoingTelemetryBufffer()
{
    return this->outgoingTelemetryBufffer;
}