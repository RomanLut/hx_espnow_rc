#pragma once

#include <Arduino.h>

#include "HX_ESPNOW_RC_Common.h"

//=====================================================================
//=====================================================================
class HXRCTransmitterStats
{
private:     

    void reset();
    
    void onPacketSendSuccess( uint8_t telemetryLength );
    void onPacketSendError();
    void onPacketSend( unsigned long timeMs );
    void onPacketSendMiss( uint16_t missedPackets );

    void update();

    friend class HXRCTransmitter;
    friend class HXRCReceiver;

public:
    uint16_t packetsSentTotal;
    uint16_t packetsSentSuccess;
    uint16_t packetsSentError;
    uint16_t packetsNotSentInTime;  //packets not sent in time because HXRCLoop() was not called in time

    unsigned long lastSendTimeMs;
    unsigned long lastSuccessfulPacketMs;

    uint16_t RSSIPacketsSentSuccess;
    uint16_t RSSIPacketsSentError;
    unsigned long RSSIupdateMs;
    uint8_t RSSIlast;

    uint32_t telemetryBytesSentTotal;
    uint32_t lastTelemetryBytesSentSpeed;
    uint32_t lastTelemetryBytesSentTotal;
    unsigned long telemetrySpeedUpdateMs;

    HXRCTransmitterStats();

    bool isFailsafe();
    uint8_t getRSSI();
    void printStats();
    
    uint32_t getTelemetrySendSpeed();

};
