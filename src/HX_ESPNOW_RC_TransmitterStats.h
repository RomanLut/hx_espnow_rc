#pragma once

#include <Arduino.h>

#include "HX_ESPNOW_RC_Common.h"

class HXRCTransmitterStats
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
    uint16_t packetsSentTotal;
    uint16_t packetsSentSuccess;
    uint16_t packetsSentError;
    uint16_t packetsNotSentInTime;  //packets not sent in time because akknowledgement took too long OR HXRCLoop() was not called in time

    //each bit contains 1 if packet was sent successfuly and in time.
    //bit  is most recent packet
    //32 bits are enought to track last 32 packets, or 32 / 20 = ~1.5 seconds. 
    //Failsafe period is 1 second.
    uint32_t sentPacketsMask;
    unsigned long lastSendTimeMs;

    uint32_t telemetryBytesSentTotal;

    uint32_t lastTelemetryBytesSentSpeed;
    uint32_t lastTelemetryBytesSentTotal;
    uint32_t lastTelemetryBytesSentTotalMs;

    HXRCTransmitterStats()
    {
        reset();
    }

    bool isFailsafe()    
    {
        //all last 20 packets are not sent?
        return (this->sentPacketsMask & 0xfffff) == 0;
    }


    //(percentage of packets sent with aknowledgement) 0...100
    uint8_t getTransmitterRSSI()
    {
        return calculateRSSI( this->sentPacketsMask, FAILSAFE_PACKETS_COUNT);
    }
    
    void reset()
    {
        this->packetsSentTotal = 0;
        this->packetsSentSuccess = 0;
        this->packetsSentError = 0;
        this->packetsNotSentInTime = 0;

        this->sentPacketsMask = 0;
        this->lastSendTimeMs = millis();
        this->telemetryBytesSentTotal = 0;

        this->lastTelemetryBytesSentSpeed = 0;
        this->lastTelemetryBytesSentTotal = 0;
        this->lastTelemetryBytesSentTotalMs = millis();
    }

    void onPacketSendSuccess()
    {
        this->packetsSentSuccess++;
        this->sentPacketsMask <<= 1;
        this->sentPacketsMask |= 1;
    }

    void onPacketSendError()
    {
        this->packetsSentError++;
        this->sentPacketsMask <<= 1;
    }

    void onPacketSendMiss()
    {
        this->packetsNotSentInTime++;
        this->sentPacketsMask <<= 1;
    }

    //telemetry send speed stats, bytes/sec
    uint32_t getTelemetrySendSpeed()
    {
        uint32_t delta = millis() - this->lastTelemetryBytesSentTotalMs;

        if ( delta > 1000 )
        {
            this->lastTelemetryBytesSentTotalMs = millis();
            this->lastTelemetryBytesSentSpeed = ((uint32_t)(this->telemetryBytesSentTotal - this->lastTelemetryBytesSentTotal))*1000 / delta;
            this->lastTelemetryBytesSentTotal = this->telemetryBytesSentTotal;
        }

        return this->lastTelemetryBytesSentSpeed;
    }
};
