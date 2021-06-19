#include "HX_ESPNOW_RC_Master.h"

HXRCMaster* HXRCMaster::pInstance;

#if defined(ESP8266)
void HXRCMaster::OnDataSentStatic(uint8_t *mac_addr, uint8_t status) {HXRCMaster::pInstance->OnDataSent( mac_addr, status );};
void HXRCMaster::OnDataRecvStatic(uint8_t *mac, uint8_t *incomingData, uint8_t len) {HXRCMaster::pInstance->OnDataRecv( mac, incomingData, len);};
#elif defined (ESP32)
void HXRCMaster::OnDataSentStatic(const uint8_t *mac_addr, esp_now_send_status_t status) {HXRCMaster::pInstance->OnDataSent( mac_addr, status );};
void HXRCMaster::OnDataRecvStatic(const uint8_t *mac, const uint8_t *incomingData, int len) {HXRCMaster::pInstance->OnDataRecv( mac, incomingData, len);};
#endif

//=====================================================================
//=====================================================================
HXRCMaster::HXRCMaster()
{
    pInstance = this;
}

//=====================================================================
//=====================================================================
HXRCMaster::~HXRCMaster()
{
}

//=====================================================================
//=====================================================================
// Callback when data is sent
#if defined(ESP8266)
void HXRCMaster::OnDataSent(uint8_t *mac_addr, uint8_t status)
#elif defined(ESP32)
void HXRCMaster::OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
#endif
{
    if( status == ESP_NOW_SEND_SUCCESS )
    {
        transmitterStats.onPacketSendSuccess( outgoingData.length );
        senderState = HXRCSS_READY_TO_SEND;
    }
    else    
    {
        transmitterStats.onPacketSendError();
        senderState = HXRCSS_RETRY_SEND;
    }
}

//=====================================================================
//=====================================================================
// Callback when data is received
#if defined(ESP8266)
void HXRCMaster::OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len)
#elif defined (ESP32)
void HXRCMaster::OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
#endif
{
    const HXRCSlavePayload* pPayload = ( const HXRCSlavePayload*) incomingData;

    if ( ( len >= HXRC_SLAVE_PAYLOAD_SIZE_BASE ) && ( len == HXRC_SLAVE_PAYLOAD_SIZE_BASE + pPayload->length ) )
    {
        if ( pPayload->checkCRC( this->config) )
        {
            if ( receiverStats.onPacketReceived( pPayload->sequenceId, pPayload->rssi, pPayload->length ) )
            {
                if ( !this->incomingTelemetryBuffer.send( pPayload->data, pPayload->length ) )
                {
                    receiverStats.onTelemetryOverflow();
                }
            }
        }
        else
        {
            receiverStats.onPacketCRCError();
        }
    }
    else
    {
        //ignore too short payload
        //Serial.println("HXRC: Error: invalid payload length");
        receiverStats.onInvalidLengthPacket();
    }
}

//=====================================================================
//=====================================================================
bool HXRCMaster::init( HXRCConfig config )
{
    this->config = config;

    transmitterStats.reset();
    receiverStats.reset();

    HXRCInitLedPin(config);

    outgoingData.sequenceId = 0;
    for ( int i = 0; i < HXRC_CHANNELS; i++ )
    {
        setChannelValue( i, 1000);
    }
    senderState = HXRCSS_READY_TO_SEND;

    if ( !HXRCInitEspNow( config ) )
    {
        return false;
    }

    esp_now_register_send_cb(OnDataSentStatic);
    esp_now_register_recv_cb(OnDataRecvStatic);

    return true;
}

//=====================================================================
//=====================================================================
void HXRCMaster::loop()
{
    unsigned long t = millis();
    unsigned long deltaT = t - transmitterStats.lastSendTimeMs;

    if ( senderState == HXRCSS_READY_TO_SEND )
    {
        int count = deltaT / PACKET_SEND_PERIOD_MS;
        if ( count > 1)
        {
            outgoingData.sequenceId += count - 1;
            transmitterStats.onPacketSendMiss( count - 1 );           
        }

        outgoingData.sequenceId++;
        outgoingData.length = outgoingTelemetryBuffer.receiveUpTo( HXRC_MASTER_TELEMETRY_SIZE_MAX, outgoingData.data );
    }

    //if state is senderState == HXRCSS_RETRY_SEND, send the same telemetry data again
    if ( senderState == HXRCSS_READY_TO_SEND || senderState == HXRCSS_RETRY_SEND )
    {
        memcpy( &outgoingData.channels, &channels, sizeof (HXRCChannels));

        outgoingData.setCRC( this->config );
        transmitterStats.onPacketSend( t );
        esp_err_t result = esp_now_send(this->config.peer_mac, (uint8_t *) &outgoingData, HXRC_MASTER_PAYLOAD_SIZE_BASE + outgoingData.length );
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
void HXRCMaster::setChannelValue(uint8_t index, uint16_t data)
{
    this->channels.setChannelValue( index, data );
}

//=====================================================================
//=====================================================================
HXRCTransmitterStats& HXRCMaster::getTransmitterStats() 
{
    return transmitterStats;
}

//=====================================================================
//=====================================================================
HXRCReceiverStats& HXRCMaster::getReceiverStats() 
{
    return receiverStats;
}

//=====================================================================
//=====================================================================
void HXRCMaster::updateLed()
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
uint16_t HXRCMaster::getIncomingTelemetry(uint16_t maxSize, uint8_t* pBuffer)
{
    return this->incomingTelemetryBuffer.receiveUpTo( maxSize, pBuffer);
}

//=====================================================================
//=====================================================================
bool HXRCMaster::sendOutgoingTelemetry( uint8_t* ptr, uint16_t size )
{
    return this->outgoingTelemetryBuffer.send( ptr, size );
}

