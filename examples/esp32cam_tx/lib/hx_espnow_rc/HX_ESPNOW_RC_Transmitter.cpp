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

    if ( len >= HXRC_SLAVE_PAYLOAD_SIZE_BASE &&  pPayload->length == HXRC_SLAVE_PAYLOAD_SIZE_BASE + pPayload->length )
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
        //Serial.println("HXRC: Error: invalid payload length");
    }
}

//=====================================================================
//=====================================================================
void HXRCTransmitter::init( HXRCConfig config )
{
    this->incomingTelemetryCallback = NULL;
    this->outgoingTelemetryCallback = NULL;

    this->config = config;

    transmitterStats.reset();
    receiverStats.reset();

    if ( this->config.ledPin != -1 )
    {
        pinMode( this->config.ledPin, OUTPUT );
        digitalWrite( this->config.ledPin, this->config.ledPinInverted ? HIGH : LOW );
    }

    outgoingData.sequenceId = 0;
    for ( int i =0; i < HXRC_CHANNELS; i++ )
    {
        setChannelValue( i, 1000);
    }
    senderState = HXRCSS_READY_TO_SEND;

#if defined(ESP8266)
    
    WiFi.mode(WIFI_STA);

    //promiscous mode is required to set channel
    wifi_promiscuous_enable(true);

    //Channel setting works only in WIFI_STA mode. Mode can be changed later.Channel will be preserved.
    if  (!wifi_set_channel( this->config.wifi_channel ) )
    {
        Serial.println("HXRC: Error: Failed to set channel");
        return;
    }
    wifi_promiscuous_enable(false);

    //review: do we need to disable wifi sleep somehow?
    //esp_wifi_set_ps(WIFI_PS_NONE);

#elif defined(ESP32)

    WiFi.mode(WIFI_STA);

    if ( esp_wifi_set_channel( this->config.wifi_channel, WIFI_SECOND_CHAN_NONE) != ESP_OK )
    {
        Serial.println("HXRC: Error: Failed to set channel");
        return;
    }

    if ( esp_wifi_set_protocol (WIFI_IF_STA, WIFI_PROTOCOL_11B) != ESP_OK)
    {
        Serial.println("HXRC: Error: Failed to enable LR mode");
        return;
    }

    /*

#include "esp_private/wifi.h"
ESP_ERROR_CHECK(esp_wifi_start());
wifi_interface_t ifx = WIFI_IF_AP;
ESP_ERROR_CHECK(esp_wifi_internal_set_fix_rate(ifx, true, WIFI_PHY_RATE_1M_L));

WIFI_PHY_RATE_LORA_250K

If you want to use esp_wifi_internal_set_fix_rate, please disable WiFi AMPDU TX by:

make menuconfig => components => Wi-Fi => Disable TX AMPDU.

*/
    esp_wifi_set_ps(WIFI_PS_NONE);

#endif

    Serial.println("HXESPNOWRC: Info: Board MAC address:");
    Serial.println(WiFi.macAddress());
    //Serial.println(WiFi.softAPmacAddress());

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK)
    {
        Serial.println("HXESPNOWRC: Error: Error initializing ESP-NOW");
        return;
    }

#if defined(ESP8266)

    if ( esp_now_set_self_role(ESP_NOW_ROLE_COMBO) != ESP_OK )
    {
        Serial.println("HXRC: Error: Failed to set role");
        return;
    }

    esp_now_add_peer(config.peer_mac, ESP_NOW_ROLE_COMBO, config.wifi_channel, NULL, 0);

#elif defined(ESP32)

    // Register peer
    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, this->config.peer_mac, 6);
    peerInfo.channel = this->config.wifi_channel;
    peerInfo.encrypt = false;
    peerInfo.ifidx = WIFI_IF_STA;

    // Add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("HXRC: Error: Failed to add peer");
        return;
    }

#endif


    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    esp_now_register_send_cb(OnDataSentStatic);

    // Register for a callback function that will be called when data is received
    esp_now_register_recv_cb(OnDataRecvStatic);

    //Serial.print("Packetsize: ");
    //Serial.println(sizeof(HXRCPayloadMaster));
    
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
HXRCRingBufer<HXRC_TELEMETRY_BUFFER_SIZE> HXRCTransmitter::getIncomingTelemetryBufffer()
{
    return this->incomingTelemetryBufffer;
}

//=====================================================================
//=====================================================================
HXRCRingBufer<HXRC_TELEMETRY_BUFFER_SIZE> HXRCTransmitter::getOutgoingTelemetryBufffer()
{
    return this->outgoingTelemetryBufffer;
}