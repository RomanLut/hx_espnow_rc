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
    const HXRCPayloadFromReceiver* pPayload = ( const HXRCPayloadFromReceiver*) incomingData;

    if ( len >= HXRC_RECEIVER_PAYLOAD_SIZE_BASE )
    {
        receiverStats.onPacketReceived( pPayload->sequenceId, pPayload->rssi, pPayload->length );
        //todo:copy telemetry data pPayloadTelemetry.length               
    }
    else
    {
        //ignore too short payload
    }
}

//=====================================================================
//=====================================================================
void HXRCTransmitter::init( HXRCConfig config )
{
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
    
    WiFi.mode(WIFI_AP);

#elif defined(ESP32)
    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_AP);

    if ( esp_wifi_set_protocol (WIFI_IF_AP, WIFI_PROTOCOL_11B) != ESP_OK)
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

    WiFi.softAP("hxrct", NULL, this->config.wifi_channel);

#endif

    WiFi.softAP("hxrct", NULL, config.wifi_channel);

    Serial.println("HXESPNOWRC: Info: Board MAC address:");
    //Serial.println(WiFi.macAddress());
    Serial.println(WiFi.softAPmacAddress());

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

    esp_now_add_peer(config.peer_mac, ESP_NOW_ROLE_SLAVE, config.wifi_channel, NULL, 0);

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


    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    esp_now_register_send_cb(OnDataSentStatic);

    // Register for a callback function that will be called when data is received
    esp_now_register_recv_cb(OnDataRecvStatic);

    //Serial.print("Packetsize: ");
    //Serial.println(sizeof(HXRCPayloadTransmitter));
    
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
            outgoingData.length = HXRC_TRANSMITTER_TELEMETRY_SIZE_MAX;
        }

        transmitterStats.onPacketSend( t );

        outgoingData.sequenceId++;
        esp_err_t result = esp_now_send( this->config.peer_mac, (uint8_t *) &outgoingData, HXRC_TRANSMITTER_PAYLOAD_SIZE_BASE + outgoingData.length);
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

