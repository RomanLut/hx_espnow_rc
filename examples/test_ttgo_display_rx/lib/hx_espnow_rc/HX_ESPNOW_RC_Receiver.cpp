#if defined (ESP8266)
#include <interrupts.h>
#endif

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
HXRCReceiver::HXRCReceiver()
{
    HXRCReceiver::pInstance = this;
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
HXRCReceiver::~HXRCReceiver()
{
#if defined (ESP32)
    vSemaphoreDelete(this->channelsMutex);
#endif
}


//=====================================================================
//=====================================================================
// Callback when data is sent
#if defined(ESP8266)
void HXRCReceiver::OnDataSent(uint8_t *mac_addr, uint8_t status)
#elif defined (ESP32)
void HXRCReceiver::OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
#endif
{
    if( status == ESP_NOW_SEND_SUCCESS )
    {
        transmitterStats.onPacketSendSuccess( outgoingTelemetry.length );
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
void HXRCReceiver::OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len)
#elif defined (ESP32)
void HXRCReceiver::OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
#endif
{
    if ( len >= HXRC_MASTER_PAYLOAD_SIZE_BASE )
    {
        const HXRCPayloadMaster* pPayload = (const HXRCPayloadMaster*) incomingData;
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
        receiverStats.onPacketReceived( pPayload->sequenceId, -2, pPayload->length );

        uint8_t lenToWrite = pPayload->length;
        if ( lenToWrite > 0 )
        {
             if ( !this->incomingTelemetryBuffer.send( pPayload->data, lenToWrite ) )
             {
                receiverStats.onTelemetryOverflow();
             }
        }
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
    receivedChannels.init();

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
        outgoingTelemetry.length = 0;
        uint8_t freeBytes = HXRC_SLAVE_TELEMETRY_SIZE_MAX;
        while ( freeBytes > 0 )
        {
            uint16_t returnedSize = outgoingTelemetryBuffer.receiveUpTo( freeBytes, &outgoingTelemetry.data[outgoingTelemetry.length] );
            if ( returnedSize == 0 ) break;
            outgoingTelemetry.length += returnedSize;
            freeBytes -= returnedSize;
        }
    }

    //if state is senderState == HXRCSS_RETRY_SEND, send the same telemetry data again
    if ( senderState == HXRCSS_READY_TO_SEND || senderState == HXRCSS_RETRY_SEND )
    {
        outgoingTelemetry.sequenceId++;
        outgoingTelemetry.rssi = receiverStats.getRSSI();

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
HXRCChannels HXRCReceiver::getChannels()
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

//=====================================================================
//=====================================================================
uint16_t HXRCReceiver::getIncomingTelemetry(uint16_t maxSize, uint8_t* pBuffer)
{
    return this->incomingTelemetryBuffer.receiveUpTo( maxSize, pBuffer);
}

//=====================================================================
//=====================================================================
bool HXRCReceiver::sendOutgoingTelemetry( uint8_t* ptr, uint16_t size )
{
    return outgoingTelemetryBuffer.send( ptr, size );
}
