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
HXRCSlave::HXRCSlave()
{
    HXRCSlave::pInstance = this;
#if defined(ESP32)
    this->channelsMutex = xSemaphoreCreateMutex();
    if( this->channelsMutex == NULL )
    {
        Serial.print("HXRC: Failed create mutex");
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
//This function wokrs in Wifi task, which may run on the second core parallel to loop task.
//We have to use thread-safe ring buffer and mutex.
#if defined(ESP8266)
void HXRCSlave::OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len)
#elif defined (ESP32)
void HXRCSlave::OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
#endif
{
    const HXRCMasterPayload* pPayload = (const HXRCMasterPayload*) incomingData;

    if ( ( len >= HXRC_MASTER_PAYLOAD_SIZE_BASE ) && ( len == HXRC_MASTER_PAYLOAD_SIZE_BASE + pPayload->length ) )
    {
        if ( pPayload->checkCRC( this->config) )
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
            if ( receiverStats.onPacketReceived( pPayload->sequenceId, -2, pPayload->length ) )
            {
                if ( !this->incomingTelemetryBuffer.send( pPayload->data, pPayload->length ) )  //length = 0 is ok
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
        //ignore unknown packet, too short
        //Serial.print("Unknown packet length:");
        //Serial.println(len);
        receiverStats.onInvalidLengthPacket();
    }
}

//=====================================================================
//=====================================================================
bool HXRCSlave::init( HXRCConfig config )
{
    this->config = config;

    transmitterStats.reset();
    receiverStats.reset();

    HXRCInitLedPin(config);

    outgoingData.sequenceId = 0;
    receivedChannels.init();

    senderState = HXRCSS_READY_TO_SEND;

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

    if ( senderState == HXRCSS_READY_TO_SEND )
    {
        outgoingData.sequenceId++;
        outgoingData.length = outgoingTelemetryBuffer.receiveUpTo( HXRC_SLAVE_TELEMETRY_SIZE_MAX, outgoingData.data );
    }

    //if state is senderState == HXRCSS_RETRY_SEND, send the same telemetry data again
    if ( senderState == HXRCSS_READY_TO_SEND || senderState == HXRCSS_RETRY_SEND )
    {
        outgoingData.rssi = receiverStats.getRSSI();

        outgoingData.setCRC(this->config);
        transmitterStats.onPacketSend( t );
        esp_err_t result = esp_now_send(this->config.peer_mac, (uint8_t *) &outgoingData, HXRC_SLAVE_PAYLOAD_SIZE_BASE + outgoingData.length );
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
        Serial.println("HXRC: Failed to get mutex");
    }
    memcpy(&ret, &receivedChannels, sizeof(receivedChannels));
    xSemaphoreGive( this->channelsMutex);
#endif

    return ret;
}

//=====================================================================
//=====================================================================
HXRCReceiverStats& HXRCSlave::getReceiverStats() 
{
    return receiverStats;
}

//=====================================================================
//=====================================================================
HXRCTransmitterStats& HXRCSlave::getTransmitterStats() 
{
    return transmitterStats;
}

//=====================================================================
//=====================================================================
void HXRCSlave::updateLed()
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
uint16_t HXRCSlave::getIncomingTelemetry(uint16_t maxSize, uint8_t* pBuffer)
{
    return this->incomingTelemetryBuffer.receiveUpTo( maxSize, pBuffer);
}

//=====================================================================
//=====================================================================
bool HXRCSlave::sendOutgoingTelemetry( uint8_t* ptr, uint16_t size )
{
    return outgoingTelemetryBuffer.send( ptr, size );
}
