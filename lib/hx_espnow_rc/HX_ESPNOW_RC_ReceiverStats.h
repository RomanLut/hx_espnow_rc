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

    bool onPacketReceived( uint16_t packetId, uint16_t sequenceId, uint8_t telemetrySize, uint8_t RSSIDbm, uint8_t noiseFloor );
    void onTelemetryOverflow();

    friend class HXRCBase;
    friend class HXRCMaster;
    friend class HXRCSlave;

public:
    unsigned long lastReceivedTimeMs;

    uint16_t prevPacketId;
    uint16_t prevSequenceId;
    //total number of packets recevied (excluding invalid/crc)
    uint16_t packetsReceived; 
    //total number of packets not received (we find it out from packetId)
    uint16_t packetsLost;

    unsigned long RSSIUpdateMs;
    uint16_t RSSIPacketsReceived;
    uint16_t RSSIPacketsLost;
    uint16_t RSSILast4;  //filtered RSSI over 4 seconds

    //number of packers with non-increased sequense id   
    uint16_t packetsRetransmit;

    uint16_t packetsCRCError;
    uint16_t packetsInvalid;
    uint32_t telemetryBytesReceivedTotal;

    uint32_t lastTelemetryBytesReceivedSpeed;
    uint32_t lastTelemetryBytesReceivedTotal;
    unsigned long telemetrySpeedUpdateMs;

    uint16_t telemetryOverflowCount;

    uint8_t remoteRSSIDbm;
    uint8_t remoteNoiseFloor;

    HXRCReceiverStats();

    bool isFailsafe();
    uint8_t getRSSI();
    //The following values: 
    //1) are awailable on Master only.
    //2) are wailable only if slave is based on ESP32.
    //3) are 0 if receiver is based on ESP8266
    uint8_t getRemoteRSSIDbm();  //RSSI in Dbm on slave. 70 means -70Dbm
    uint8_t getRemoteNoiseFloor();  //Noise floor on slave. 90 means - 90Dbm
    uint8_t getRemoteSNR(); //Signal to noise ratio on slave in Db.

    void printStats();
    
    uint32_t getTelemetryReceivedSpeed();

    void onInvalidPacket();
    void onPacketCRCError();

};
