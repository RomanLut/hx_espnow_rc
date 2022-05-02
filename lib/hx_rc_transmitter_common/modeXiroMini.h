#pragma once

#include <Arduino.h>

#include <stdio.h>
#include <stdarg.h>

#include "modeBase.h"

//=====================================================================
//=====================================================================
class ModeXiroMini : public ModeBase
{
private:
    unsigned long lastStats;
    unsigned long lastPacketTime;
    unsigned long lastRTPPacketTime;
    unsigned long  lastRTPTrigger;
    bool connected;
    uint8_t batPercentage;
    uint8_t numSats;
    uint16_t height;

    uint32_t cmdPacketsSent;
    uint32_t cmdPacketsReceived;
    uint32_t rtpPacketsTotal;
    uint32_t cmdPacketsMirrored;
    uint32_t rtpPacketsMirrored;

    void sendChannels(const HXChannels* channels);
    void fillOutgoingTelemetry(HC06Interface* externalBTSerial);
    void processIncomingTelemetry(HC06Interface* externalBTSerial);
    void readPackets();
    void udpWriteWithChecksum( uint8_t value, uint8_t* checksum );
    void udpWriteSwitchWithChecksum( const HXChannels* channels, uint8_t channelIndex, uint16_t minValue, uint8_t* checksum );
    void startVideo();

public:
    static ModeXiroMini instance;
    static const char* name;

    ModeXiroMini();

    void start( JsonDocument* json );

    void loop(
        const HXChannels* channels,
        HC06Interface* externalBTSerial,
        Smartport* sport
    );
};


