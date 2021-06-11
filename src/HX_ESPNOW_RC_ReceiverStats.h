#pragma once

#include <Arduino.h>
#include "HX_ESPNOW_RC_Common.h"

class HXRCReceiverStats
{
private:

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

    HXRCReceiverStats()
    {
        reset();
    }

    void reset()
    {
        unsigned long t = millis();
        
        this->lastReceivedTimeMs = t - FAILSAFE_PERIOD_MS;

        this->prevSequenceId = 0;
        this->packetsSuccess = 0;
        this->packetsError = 0;

        this->RSSIUpdateMs = t;
        this->RSSIPacketsSuccess = 0;
        this->RSSIPacketsError = 0;
        this->RSSILast = 0;

        this->remoteReceiverRSSI = -1;
        this->telemetryBytesReceivedTotal = 0;

        this->lastTelemetryBytesReceivedSpeed = 0;
        this->lastTelemetryBytesReceivedTotal = 0;
        this->telemetrySpeedUpdateMs = t;
    }

    bool isFailsafe()    
    {
        unsigned long delta = millis() - this->lastReceivedTimeMs;
        return delta >= FAILSAFE_PERIOD_MS;
    }

    //(percentage of packets received) 0...100
    //or -1 if not received for a long time
    uint8_t getRSSI()
    {
        unsigned long t = millis();
        unsigned long delta = t - this->RSSIUpdateMs;

        if ( delta > 1000)
        {
            uint16_t packetsSuccessCount = this->packetsSuccess - this->RSSIPacketsSuccess;
            uint16_t packetsErrorCount = this->packetsError - this->RSSIPacketsError;
            uint16_t totalPackets = packetsSuccessCount + packetsErrorCount;

            this->RSSILast = ( totalPackets > 0 ) ? (((uint32_t)packetsSuccessCount) * 100 / totalPackets) : 0;
            this->RSSIPacketsSuccess = this->packetsSuccess;
            this->RSSIPacketsError = this->packetsError;
            this->RSSIUpdateMs = t; 
        }
        return this->RSSILast;


    }

    int8_t getRemoteReceiverRSSI()
    {
        if ( isFailsafe() )
        {
            return -1;
        }

        return this->remoteReceiverRSSI;
    }


    void onPacketReceived( uint16_t sequenceId, int8_t rssi, uint8_t telemetrySize )
    {
        this->remoteReceiverRSSI = rssi;
        this->telemetryBytesReceivedTotal += telemetrySize;

        uint16_t seqDelta = sequenceId - this->prevSequenceId;

        if ( seqDelta > 1)
        {
            this->packetsError += seqDelta - 1;
        }
        
        this->packetsSuccess++;

        this->prevSequenceId = sequenceId;
        this->lastReceivedTimeMs = millis();
    }

    //telemetry receive speed stats, bytes/sec
    uint32_t getTelemetryReceivedSpeed()
    {
        unsigned long t = millis();
        uint32_t delta = t - this->telemetrySpeedUpdateMs;

        if ( delta > 1000 )
        {
            this->lastTelemetryBytesReceivedSpeed = ((uint32_t)(this->telemetryBytesReceivedTotal - this->lastTelemetryBytesReceivedTotal)) * 1000 / delta;
            this->lastTelemetryBytesReceivedTotal = this->telemetryBytesReceivedTotal;
            this->telemetrySpeedUpdateMs = t;
        }

        return this->lastTelemetryBytesReceivedSpeed;
    }

    void update()
    {
        getTelemetryReceivedSpeed(); 
        getRSSI(); 
    }

};
