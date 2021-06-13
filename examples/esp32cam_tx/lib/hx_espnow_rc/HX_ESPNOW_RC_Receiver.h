#pragma once

#include <Arduino.h>

#include "HX_ESPNOW_RC_Common.h"
#include "HX_ESPNOW_RC_ReceiverStats.h"
#include "HX_ESPNOW_RC_TransmitterStats.h"

//=====================================================================
//=====================================================================
class HXRCReceiver
{
private:

    HXRCConfig config;

    static HXRCReceiver* pInstance;

    HXRCTransmitterStats transmitterStats;
    HXRCReceiverStats receiverStats;

    HXRCSenderStateEnum senderState;

    HXRCChannels receivedChannels;

    HXRCPayloadSlave outgoingTelemetry;
#if defined(ESP8266)
    static void OnDataSentStatic(uint8_t *mac_addr, uint8_t status);
    static void OnDataRecvStatic(uint8_t *mac, uint8_t *incomingData, uint8_t len);
    void OnDataSent(uint8_t *mac_addr, uint8_t status);
    void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len);
#elif defined (ESP32)
    static void OnDataSentStatic(const uint8_t *mac_addr, esp_now_send_status_t status);
    static void OnDataRecvStatic(const uint8_t *mac, const uint8_t *incomingData, int len);
    void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
    void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);
#endif

    void updateLed();

public:

//TODO: settings:
//Receiver MAC address
//channel

    HXRCReceiver()
    {
        HXRCReceiver::pInstance = this;
    }

    void init( HXRCConfig config );
    void loop();

    //index = 0..15
    //data = 1000...2000
    uint16_t getChannelValue( uint8_t index );

    HXRCReceiverStats& getReceiverStats();
    HXRCTransmitterStats& getTransmitterStats();
};

