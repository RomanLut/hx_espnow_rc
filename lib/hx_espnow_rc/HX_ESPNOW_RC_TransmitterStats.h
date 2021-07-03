#pragma once

#include <Arduino.h>

#include "HX_ESPNOW_RC_Common.h"

//=====================================================================
//=====================================================================
class HXRCTransmitterStats
{
private:     

    void reset();
    
    void onPacketSendError();
    void onPacketSend( unsigned long timeMs );
    void onPacketSendMiss( uint16_t missedPackets );
    void onPacketAck( uint8_t telemetryLength );

    void update();

    friend class HXRCBase;
    friend class HXRCMaster;
    friend class HXRCSlave;

public:
    //number of packets send, including not sent due to API error
    uint16_t packetsSentTotal;
    uint16_t packetsAcknowledged;
    //API error
    uint16_t packetsSentError;
    //packets not sent in time because HXRCLoop() was not called in time
    uint16_t packetsNotSentInTime;  

    unsigned long lastSendTimeMs;
    unsigned long lastAcknowledgedPacketMs;

    uint16_t RSSIPacketsAcknowledged;
    uint16_t RSSIPacketsTotal;
    unsigned long RSSIUpdateMs;
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
