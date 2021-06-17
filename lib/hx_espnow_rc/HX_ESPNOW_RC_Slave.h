#pragma once

#include <Arduino.h>

#include "HX_ESPNOW_RC_Common.h"
#include "HX_ESPNOW_RC_RingBuffer.h"
#include "HX_ESPNOW_RC_ReceiverStats.h"
#include "HX_ESPNOW_RC_TransmitterStats.h"

//=====================================================================
//=====================================================================
class HXRCSlave
{
private:

    HXRCConfig config;

    static HXRCSlave* pInstance;

    HXRCTransmitterStats transmitterStats;
    HXRCReceiverStats receiverStats;

    volatile HXRCSenderStateEnum senderState;

    HXRCRingBuffer<HXRC_TELEMETRY_BUFFER_SIZE> incomingTelemetryBuffer;
    HXRCRingBuffer<HXRC_TELEMETRY_BUFFER_SIZE> outgoingTelemetryBuffer;

#if defined(ESP32)
    SemaphoreHandle_t channelsMutex;
#endif    
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

    HXRCSlave();
    ~HXRCSlave();

    bool init( HXRCConfig config );
    void loop();

    //index = 0..15
    //data = 1000...2000
    HXRCChannels getChannels();

    //return portion of incoming telemetry into buffer pBuffer which has size maxSize
    //returns size of returned data
    uint16_t getIncomingTelemetry(uint16_t maxSize, uint8_t* pBuffer);
    
    //add size byter from *ptr buffer to outgoing telemetry stream
    //returns true if bytes where added sucessfully
    //return false if buffer is overflown
    //As packet sensing is done from loop thread, 
    //we can send at most HXRC_SLAVE_TELEMETRY_SIZE_MAX every loop.
    bool sendOutgoingTelemetry( uint8_t* ptr, uint16_t size );

    HXRCReceiverStats& getReceiverStats();
    HXRCTransmitterStats& getTransmitterStats();
};

