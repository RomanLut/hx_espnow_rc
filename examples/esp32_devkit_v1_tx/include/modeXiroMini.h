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
    bool connected;
    uint8_t batPercentage;
    uint8_t numSats;
    uint16_t height;

    void sendChannels(HXSBUSDecoder* sbusDecoder);
    void fillOutgoingTelemetry(HC06Interface* externalBTSerial);
    void processIncomingTelemetry(HC06Interface* externalBTSerial);
    void readPackets();
    void udpWriteWithChecksum( uint8_t value, uint8_t* checksum );
    void udpWriteSwitchWithChecksum( HXSBUSDecoder* sbusDecoder, uint8_t channelIndex, uint8_t* checksum );

public:
    static ModeXiroMini instance;

    ModeXiroMini();

    void start();

    void loop(
        HXSBUSDecoder* sbusDecoder,
        HC06Interface* externalBTSerial,
        Smartport* sport
    );
};


