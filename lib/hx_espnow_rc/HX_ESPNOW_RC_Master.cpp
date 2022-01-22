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
HXRCMaster::HXRCMaster() : HXRCBase()
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
    if( status != ESP_NOW_SEND_SUCCESS )
    {
        transmitterStats.onPacketSendError();
    }
    senderState = HXRCSS_READY_TO_SEND;
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

    if ( 
        ( len >= HXRC_SLAVE_PAYLOAD_SIZE_BASE ) && 
        ( len == HXRC_SLAVE_PAYLOAD_SIZE_BASE + pPayload->length ) && 
        ( pPayload->key == config.key )
        )
    {
        if ( pPayload->checkCRC() )
        {
            this->lastReceived = millis();

            this->A1 = pPayload->A1;
            this->A2 = pPayload->A2;

            memcpy( this->peerMac, mac, 6 );
#if defined(ESP32)
            memcpy( capture.peerMac, mac, 6 );
#endif            

            if ( receiverStats.onPacketReceived( pPayload->packetId, pPayload->sequenceId, pPayload->length, pPayload->RSSIDbm, pPayload->NoiseFloor ) )
            {
                if ( !this->incomingTelemetryBuffer.send( pPayload->data, pPayload->length ) )
                {
                    receiverStats.onTelemetryOverflow();
                }
            }
            this->receivedSequenceId = pPayload->sequenceId;

            if ( this->waitAck && ( outgoingData.sequenceId == pPayload->ackSequenceId ) )
            {
                this->waitAck = false;
                this->transmitterStats.onPacketAck( outgoingData.length );
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
        receiverStats.onInvalidPacket();
    }
}

//=====================================================================
//=====================================================================
bool HXRCMaster::init( HXRCConfig config )
{
    if ( !HXRCBase::init( config ) ) return false;

    this->channels.init();
    
    outgoingData.key = config.key;
    outgoingData.packetId = 0;
    outgoingData.sequenceId = 0;
    outgoingData.length = 0;

    if ( !HXRCInitEspNow( config ) )
    {
        return false;
    }

    esp_now_register_send_cb(OnDataSentStatic);
    esp_now_register_recv_cb(OnDataRecvStatic);

    this->lastReceived = 0;

    return true;
}

//=====================================================================
//=====================================================================
void HXRCMaster::loop()
{
    if ( senderState == HXRCSS_READY_TO_SEND )
    {
        unsigned long t = millis();
        unsigned long deltaT = t - transmitterStats.lastSendTimeMs;

        int count = deltaT / (this->config.LRMode ? DEFAULT_PACKET_SEND_PERIOD_LR_MS : DEFAULT_PACKET_SEND_PERIOD_MS);
        if ( count > 1)
        {
            outgoingData.packetId += count - 1;  //missed time to send packet(s) with desired rate
            transmitterStats.onPacketSendMiss( count - 1 );           
        }

        if ( count > 0 )
        {
            outgoingData.packetId++;

            //always send fresh channels values
            memcpy( &outgoingData.channels, &channels, sizeof (HXRCChannels));

            if ( !this->waitAck )
            {
                outgoingData.sequenceId++;
                outgoingData.length = outgoingTelemetryBuffer.receiveUpTo( HXRC_MASTER_TELEMETRY_SIZE_MAX, outgoingData.data );
                this->waitAck = true;
            }

            outgoingData.ackSequenceId = receivedSequenceId;

            outgoingData.setCRC();
            transmitterStats.onPacketSend( t );
            esp_err_t result = esp_now_send(BROADCAST_MAC, (uint8_t *) &outgoingData, HXRC_MASTER_PAYLOAD_SIZE_BASE + outgoingData.length );
            //esp_err_t result = esp_now_send(NULL, (uint8_t *) &outgoingData, HXRC_MASTER_PAYLOAD_SIZE_BASE + outgoingData.length );
            if (result == ESP_OK) 
            {
                senderState = HXRCSS_WAIT_SEND_FINISH;
            }
            else
            {
                Serial.println(result);
            }            
        }

    }

    HXRCBase::loop();
}

//=====================================================================
//=====================================================================
void HXRCMaster::setChannelValue(uint8_t index, uint16_t data)
{
    this->channels.setChannelValue( index, data );
}

//=====================================================================
//=====================================================================
uint32_t HXRCMaster::getA1()
{
    return this->A1;
}

//=====================================================================
//=====================================================================
uint32_t HXRCMaster::getA2()
{
    return this->A2;
}


