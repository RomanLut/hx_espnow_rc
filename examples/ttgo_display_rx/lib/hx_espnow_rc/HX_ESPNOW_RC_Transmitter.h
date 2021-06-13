#pragma once

#include <Arduino.h>

#include "HX_ESPNOW_RC_Common.h"
#include "HX_ESPNOW_RC_RingBuffer.h"
#include "HX_ESPNOW_RC_TransmitterStats.h"
#include "HX_ESPNOW_RC_ReceiverStats.h"

//TODO: settings:
//Receiver MAC address
//channel

//=====================================================================
//=====================================================================
class HXRCTransmitter
{
private:

    HXRCConfig config;

    static HXRCTransmitter* pInstance;

    HXRCTransmitterStats transmitterStats;
    HXRCReceiverStats receiverStats;
    HXRCSenderStateEnum senderState;

    HXRCPayloadMaster outgoingData;

    HXRCRingBufer<HXRC_TELEMETRY_BUFFER_SIZE> incomingTelemetryBufffer;
    HXRCRingBufer<HXRC_TELEMETRY_BUFFER_SIZE> outgoingTelemetryBufffer;

    void (*incomingTelemetryCallback)(void* parm, HXRCTransmitter& transmitter);
    void* incomingTelemetryCallbackParm;
    void (*outgoingTelemetryCallback)(void* parm, HXRCTransmitter& transmitter);
    void* outgoingTelemetryCallbackParm;

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

    HXRCTransmitter()
    {
        pInstance = this;
        this->incomingTelemetryCallback = NULL;
        this->outgoingTelemetryCallback = NULL;
    }

    bool init(HXRCConfig config);
    void loop();

    void addIncomingTelemetryCallback( void (*callback)(void* parm, HXRCTransmitter& transmitter), void* parm);
    void addOutgoingTelemetryCallback( void (*callback)(void* parm, HXRCTransmitter& transmitter), void* parm);

    //index = 0..15
    //data = 1000...2000
    void setChannelValue( uint8_t index, uint16_t data);

    HXRCRingBufer<HXRC_TELEMETRY_BUFFER_SIZE>& getIncomingTelemetryBufffer();
    HXRCRingBufer<HXRC_TELEMETRY_BUFFER_SIZE>& getOutgoingTelemetryBufffer();

    HXRCTransmitterStats& getTransmitterStats();
    HXRCReceiverStats& getReceiverStats();
};

