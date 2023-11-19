#if defined (ESP8266)
#include <interrupts.h>
#endif

#include "HX_ESPNOW_RC_Slave.h"

HXRCSlave* HXRCSlave::pInstance;

#if defined(ESP8266)
void HXRCSlave::OnDataSentStatic(uint8_t *mac_addr, uint8_t status) {HXRCSlave::pInstance->OnDataSent( mac_addr, status );};
void HXRCSlave::OnDataRecvStatic(uint8_t *mac, uint8_t *incomingData, uint8_t len) {HXRCSlave::pInstance->OnDataRecv( mac, incomingData, len);};
#elif defined (ESP32)
void HXRCSlave::OnDataSentStatic(const uint8_t *mac_addr, esp_now_send_status_t status) {HXRCSlave::pInstance->OnDataSent( mac_addr, status );};
void HXRCSlave::OnDataRecvStatic(const uint8_t *mac, const uint8_t *incomingData, int len) {HXRCSlave::pInstance->OnDataRecv( mac, incomingData, len);};
#endif

//=====================================================================
//=====================================================================
HXRCSlave::HXRCSlave() : HXRCBase()
{
    HXRCSlave::pInstance = this;
    this->gotIncomingPacket = false;
#if defined(ESP32)
    this->channelsMutex = xSemaphoreCreateMutex();
    if( this->channelsMutex == NULL )
    {
        HXRCLOG.print("HXRC: Failed create mutex");
    }
#endif
}

//=====================================================================
//=====================================================================
HXRCSlave::~HXRCSlave()
{
#if defined (ESP32)
    vSemaphoreDelete(this->channelsMutex);
#endif
}


//=====================================================================
//=====================================================================
// Callback when data is sent
#if defined(ESP8266)
void HXRCSlave::OnDataSent(uint8_t *mac_addr, uint8_t status)
#elif defined (ESP32)
void HXRCSlave::OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
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
//This function works in Wifi task, which may run on the second core parallel to loop task.
//We have to use thread-safe ring buffer and mutex.
#if defined(ESP8266)
void HXRCSlave::OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len)
#elif defined (ESP32)
void HXRCSlave::OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
#endif
{
    const HXRCMasterPayload* pPayload = (const HXRCMasterPayload*) incomingData;

    if ( 
        ( len >= HXRC_MASTER_PAYLOAD_SIZE_BASE ) && 
        ( len == HXRC_MASTER_PAYLOAD_SIZE_BASE + pPayload->length ) &&
        ( pPayload->key == config.key ) 
    )
    {
        if ( pPayload->checkCRC() )
        {
#if defined(ESP8266)
            {
                esp8266::InterruptLock lock;
                memcpy(&receivedChannels, &pPayload->channels, sizeof(receivedChannels));
            }
#elif(ESP32)
            if ( xSemaphoreTake( this->channelsMutex,  (TickType_t) 100) == pdTRUE );
            {
                memcpy(&receivedChannels, &pPayload->channels, sizeof(receivedChannels));
                xSemaphoreGive( this->channelsMutex);
            }
#endif
            memcpy( this->peerMac, mac, 6 );
#if defined(ESP32)
            memcpy( capture.peerMac, mac, 6 );
#endif            

            if ( receiverStats.onPacketReceived( pPayload->packetId, pPayload->sequenceId, pPayload->length, 0, 0  ) )
            {
                if ( !this->incomingTelemetryBuffer.send( pPayload->data, pPayload->length ) )  //length = 0 is ok
                {
                    receiverStats.onTelemetryOverflow();
                }
            }

            //there may be late packets which acknowledge older sequenceId. thay should be ignored.
            if ( this->waitAck && ( outgoingData.sequenceId == pPayload->ackSequenceId ) )
            {
                //Serial.printf("Acked:%d\n",outgoingData.sequenceId);
                this->waitAck = false;
                this->transmitterStats.onPacketAck( outgoingData.length );
            }

/*
            if ( this->waitAck && ( outgoingData.sequenceId != pPayload->ackSequenceId ) )
            {
                Serial.printf("Expected:%d, got:%d\n",outgoingData.sequenceId,pPayload->ackSequenceId);
            }
*/

            this->gotIncomingPacket = true;
        }
        else
        {
            receiverStats.onPacketCRCError();
        }
    }
    else
    {
        //ignore unknown packet, too short
        //HXRCLOG.print("Unknown packet length:");
        //HXRCLOG.println(len);
        receiverStats.onInvalidPacket();
    }

}

//=====================================================================
//=====================================================================
bool HXRCSlave::init( HXRCConfig config )
{
    if ( !HXRCBase::init( config ) ) return false;

    outgoingData.key = config.key;
    outgoingData.packetId = 0;
    outgoingData.sequenceId = 0;
    outgoingData.length = 0;

    receivedChannels.init();

    if ( !HXRCInitEspNow( config ))
    {
        return false;
    }

    esp_now_register_recv_cb(OnDataRecvStatic);
    esp_now_register_send_cb(OnDataSentStatic);

    return true;
}

//=====================================================================
//=====================================================================
void HXRCSlave::loop()
{
    unsigned long t = millis();
    unsigned long deltaT = t - transmitterStats.lastSendTimeMs;

    if (deltaT > 500)
    {
        //HXRCLOG.println("Callback timeout!");
        senderState = HXRCSS_READY_TO_SEND;
    }

    if ( senderState == HXRCSS_READY_TO_SEND )
    {
        //reply as soon as we got packet from master. 
        //we send packets in responce only to avoid collision with master packets
        if ( this->gotIncomingPacket)
        {
            this->gotIncomingPacket = false;
            outgoingData.packetId++;

            //if packet is not acked, retransmit it with the same outgoingData.sequenceId
            if ( !this->waitAck )
            {
                outgoingData.sequenceId++;
                outgoingData.length = outgoingTelemetryBuffer.receiveUpTo( config.getSlaveTelemetryPayloadSize(), outgoingData.data );
                this->waitAck = true;
            }

            outgoingData.ackSequenceId = this->receiverStats.prevSequenceId;  //ACK last received telemetry packet so other end does continue seuqence
            outgoingData.A1 = A1;
            outgoingData.A2 = A2;
            outgoingData.RSSIDbm = this->transmitterStats.getRSSIDbm();
            outgoingData.NoiseFloor = this->transmitterStats.getNoiseFloor();

            outgoingData.setCRC();
            transmitterStats.onPacketSend( t );
            senderState = HXRCSS_WAIT_SEND_FINISH;
            esp_err_t result = esp_now_send(BROADCAST_MAC, (uint8_t *) &outgoingData, HXRC_SLAVE_PAYLOAD_SIZE_BASE + outgoingData.length );
            if (result != ESP_OK) 
            {
                HXRCLOG.println("Packet send error!");
                transmitterStats.onPacketSendError();
            }            
        }

    }

    HXRCBase::loop();
}

//=====================================================================
//=====================================================================
HXRCChannels HXRCSlave::getChannels()
{
    HXRCChannels ret;

#if defined(ESP8266)
    {
        esp8266::InterruptLock lock;
        memcpy(&ret, &receivedChannels, sizeof(receivedChannels));
    }
#elif defined (ESP32)
    if ( xSemaphoreTake( this->channelsMutex,  portMAX_DELAY ) != pdTRUE )
    {
        HXRCLOG.println("HXRC: Failed to get mutex");
    }
    memcpy(&ret, &receivedChannels, sizeof(receivedChannels));
    xSemaphoreGive( this->channelsMutex);
#endif

    return ret;
}


//=====================================================================
//=====================================================================
void HXRCSlave::setA1(uint32_t value)
{
    this->A1 = value;
}

//=====================================================================
//=====================================================================
void HXRCSlave::setA2(uint32_t value)
{
    this->A2 = value;
}

