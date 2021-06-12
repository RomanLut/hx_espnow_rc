#pragma once

#include <Arduino.h>
#include "HX_ESPNOW_RC_Common.h"

//=====================================================================
//=====================================================================
class HXRCReceiverStats
{
private:

    void reset();
    void update();

    void onPacketReceived( uint16_t sequenceId, int8_t rssi, uint8_t telemetrySize );

    friend class HXRCTransmitter;
    friend class HXRCReceiver;

public:
    unsigned long lastReceivedTimeMs;

    uint16_t prevSequenceId;
    uint16_t packetsSuccess;
    uint16_t packetsError;

    //receiver RSSI of other size ( value is received with telemetry)
    //-1 if is unknown
    int8_t remoteReceiverRSSI;

    unsigned long RSSIUpdateMs;
    uint16_t RSSIPacketsSuccess;
    uint16_t RSSIPacketsError;
    uint8_t RSSILast;

    uint32_t telemetryBytesReceivedTotal;

    uint32_t lastTelemetryBytesReceivedSpeed;
    uint32_t lastTelemetryBytesReceivedTotal;
    unsigned long telemetrySpeedUpdateMs;

    HXRCReceiverStats();

    bool isFailsafe();
    uint8_t getRSSI();
    int8_t getRemoteReceiverRSSI();
    void printStats();
    
    uint32_t getTelemetryReceivedSpeed();

};
