#pragma once

#include <Arduino.h>

#include "HX_ESPNOW_RC_Common.h"

class HXRCTransmitterStats
{
public:
    uint16_t packetsSentTotal;
    uint16_t packetsSentSuccess;
    uint16_t packetsSentError;
    uint16_t packetsNotSentInTime;  //packets not sent in time because akknowledgement took too long OR HXRCLoop() was not called in time

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

    HXRCTransmitterStats()
    {
        reset();
    }

    bool isFailsafe()    
    {
        unsigned long delta = millis() - this->lastSuccessfulPacketMs;
        return delta >= FAILSAFE_PERIOD_MS;
    }

    //(percentage of packets sent with aknowledgement) 0...100
    uint8_t getRSSI()
    {
        unsigned long t = millis();
        unsigned long delta = t - this->RSSIupdateMs;

        if ( delta > 1000)
        {
            uint16_t packetsSuccessCount = this->packetsSentSuccess - this->RSSIPacketsSentSuccess;
            uint16_t packetsErrorCount = this->packetsSentError + this->packetsNotSentInTime - this->RSSIPacketsSentError;
            uint16_t totalPackets = packetsSuccessCount + packetsErrorCount;

            this->RSSIlast = ( totalPackets > 0 ) ? (((uint32_t)packetsSuccessCount) * 100 / totalPackets) : 0;
            this->RSSIPacketsSentSuccess = this->packetsSentSuccess;
            this->RSSIPacketsSentError = this->packetsSentError + this->packetsNotSentInTime;
            this->RSSIupdateMs = t; 
        }
        return this->RSSIlast;
    }
    
    void reset()
    {
        unsigned long t = millis();

        this->packetsSentTotal = 0;
        this->packetsSentSuccess = 0;
        this->packetsSentError = 0;
        this->packetsNotSentInTime = 0;

        this->lastSendTimeMs = t;
        this->lastSuccessfulPacketMs = t - FAILSAFE_PERIOD_MS;

        this->RSSIPacketsSentSuccess = 0;
        this->RSSIPacketsSentError = 0;
        this->RSSIupdateMs = t; 
        this->RSSIlast = 0;

        this->telemetryBytesSentTotal = 0;

        this->lastTelemetryBytesSentSpeed = 0;
        this->lastTelemetryBytesSentTotal = 0;
        this->telemetrySpeedUpdateMs = t;
    }

    void onPacketSendSuccess( uint8_t telemetryLength )
    {
        this->packetsSentSuccess++;
        this->telemetryBytesSentTotal += telemetryLength;
        this->lastSuccessfulPacketMs = millis();
    }

    void onPacketSendError()
    {
        this->packetsSentError++;
    }

    void onPacketSend( unsigned long timeMs )
    {
        this->lastSendTimeMs = timeMs;
        this->packetsSentTotal++;
    }

    void onPacketSendMiss( uint16_t missedPackets )
    {
        this->packetsNotSentInTime += missedPackets;
    }

    //telemetry send speed stats, bytes/sec
    uint32_t getTelemetrySendSpeed()
    {
        unsigned long t = millis();
        unsigned long delta = t - this->telemetrySpeedUpdateMs;

        if ( delta > 1000 )
        {
            this->lastTelemetryBytesSentSpeed = ((uint32_t)(this->telemetryBytesSentTotal - this->lastTelemetryBytesSentTotal))*1000 / delta;
            this->lastTelemetryBytesSentTotal = this->telemetryBytesSentTotal;
            this->telemetrySpeedUpdateMs = t;
        }

        return this->lastTelemetryBytesSentSpeed;
    }

    void update()
    {
        getTelemetrySendSpeed(); 
        getRSSI(); 
    }
};
