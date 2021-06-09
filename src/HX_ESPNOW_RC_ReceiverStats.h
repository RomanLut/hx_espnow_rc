#pragma once

#include <Arduino.h>
#include "HX_ESPNOW_RC_Common.h"

class HXRCReceiverStats
{
private:
    uint8_t calculateRSSI(uint32_t value, uint8_t bitsCount)
    {
        uint32_t bit = 1;            
        uint8_t count = 0;
        for ( uint8_t i = 0; i < bitsCount; i++ )
        {
            if ( value & bit ) count++;
            bit <<= 1;
        }

        return (uint8_t)(((uint16_t)count) * 100 / bitsCount);
    }


public:
    //== receiver stats
    //shifted with sequenceId
    uint32_t receivedPacketsMask;
    uint32_t  lastReceivedTime;

    uint16_t prevSequenceId;

    //receiver RSSI of other size ( value is received with telemetry)
    //-1 if is unknown
    int8_t remoteReceiverRSSI;

    uint16_t telemetryBytesReceivedTotal;

    uint32_t lastTelemetryBytesReceivedSpeed;
    uint32_t lastTelemetryBytesReceivedTotal;
    uint32_t lastTelemetryBytesReceivedTotalMs;

    HXRCReceiverStats()
    {
        reset();
    }

    bool isFailsafe()    
    {
        uint32_t delta = millis() - this->lastReceivedTime;
        if ( delta >= FAILSAFE_PERIOD_MS )
        {
            return true;
        }

        //all last 20 packets are not received?
        return (receivedPacketsMask & FAILSAFE_PACKETS_BITMASK) == 0;
    }


    //(percentage of packets received) 0...100
    //or -1 if not received for a long time
    uint8_t getRSSI()
    {
        uint32_t delta = millis() - this->lastReceivedTime;
        if ( delta >= FAILSAFE_PERIOD_MS )
        {
            return -1;
        }

        return calculateRSSI( this->receivedPacketsMask, FAILSAFE_PACKETS_COUNT);
    }

    int8_t getRemoteReceiverRSSI()
    {
        uint32_t delta = millis() - this->lastReceivedTime;
        if ( delta >= FAILSAFE_PERIOD_MS )
        {
            return -1;
        }

        return this->remoteReceiverRSSI;
    }

    void reset()
    {
        this->receivedPacketsMask = 0;
        this->lastReceivedTime = 0;
        this->prevSequenceId = 0;
        this->remoteReceiverRSSI = -1;
        this->telemetryBytesReceivedTotal = 0;

        this->lastTelemetryBytesReceivedSpeed = 0;
        this->lastTelemetryBytesReceivedTotal = 0;
        this->lastTelemetryBytesReceivedTotalMs = millis();
    }

    void onPacketReceived( const HXRCPayload* payload )
    {
        if ( payload->packetId == HXRCPKID_TELEMETRY )
        {
            this->remoteReceiverRSSI = (int8_t)(((const HXRCPayloadTelemetry*)payload)->rssi);
            this->telemetryBytesReceivedTotal += ((const HXRCPayloadTelemetry*)payload)->length;
        }

        uint16_t seqDelta = payload->sequenceId - this->prevSequenceId;
        if ( seqDelta > 32 ) seqDelta = 32;

        while ( seqDelta > 0)
        {
            this->receivedPacketsMask <<= 1;
            seqDelta--;
        }
        this->receivedPacketsMask |= 1;

        this->prevSequenceId = payload->sequenceId;
        this->lastReceivedTime = millis();
    }

    //telemetry receive speed stats, bytes/sec
    uint32_t getTelemetryReceivedSpeed()
    {
        uint32_t delta = millis() - this->lastTelemetryBytesReceivedTotalMs;

        if ( delta > 1000 )
        {
            this->lastTelemetryBytesReceivedTotalMs = millis();
            this->lastTelemetryBytesReceivedSpeed = ((uint32_t)(this->telemetryBytesReceivedTotal - this->lastTelemetryBytesReceivedTotal)) * 1000 / delta;
            this->lastTelemetryBytesReceivedTotal = this->telemetryBytesReceivedTotal;
        }

        return this->lastTelemetryBytesReceivedSpeed;
    }

};
