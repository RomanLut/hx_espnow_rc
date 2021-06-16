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

    //returns false if sequence id did not increase (it is retransmission)
    bool onPacketReceived( uint16_t sequenceId, int8_t rssi, uint8_t telemetrySize );
    void onTelemetryOverflow();

    friend class HXRCTransmitter;
    friend class HXRCReceiver;

public:
    unsigned long lastReceivedTimeMs;

    uint16_t prevSequenceId;
    uint16_t packetsSuccess;
    uint16_t packetsError;

    //receiver RSSI of other size ( value is received with telemetry)
    //-1 if is unknown because we are not receiving telemetry from peer
    //-2 on Slave ( this parameter is not sent from master to slave ).
    int8_t remoteReceiverRSSI;

    unsigned long RSSIUpdateMs;
    uint16_t RSSIPacketsSuccess;
    uint16_t RSSIPacketsError;
    uint8_t RSSILast;

    //number of packers with non-increased sequense id   
    //it might happend that we got packet but sender did not got ack
    //wei gnore such packets
    uint16_t packetsRetransmit;
    uint32_t telemetryBytesReceivedTotal;

    uint32_t lastTelemetryBytesReceivedSpeed;
    uint32_t lastTelemetryBytesReceivedTotal;
    unsigned long telemetrySpeedUpdateMs;

    uint16_t telemetryOverflowCount;

    HXRCReceiverStats();

    bool isFailsafe();
    uint8_t getRSSI();
    int8_t getRemoteReceiverRSSI();
    void printStats();
    
    uint32_t getTelemetryReceivedSpeed();

};
