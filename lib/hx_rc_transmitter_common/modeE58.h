#pragma once

#include <Arduino.h>

#include <stdio.h>
#include <stdarg.h>

#include "modeBase.h"

//=====================================================================
//=====================================================================
class ModeE58 : public ModeBase
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
    void sendCommand( byte x, byte y, byte z, byte rot, byte command );

public:
    static ModeE58 instance;

    ModeE58();

    void start();

    void loop(
        const HXChannels* channels,
        HC06Interface* externalBTSerial,
        Smartport* sport
    );
};


