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
    bool connected;
    uint8_t batPercentage;
    uint8_t numSats;
    uint16_t height;

    uint32_t cmdPacketsSent;
    uint32_t cmdPacketsReceived;
    uint32_t rtpPacketsTotal;
    uint32_t cmdPacketsMirrored;
    uint32_t rtpPacketsMirrored;

    void sendChannels(PPMDecoder* ppmDecoder);
    void fillOutgoingTelemetry(MavEsp8266Interface* MavEsp8266Serial);
    void processIncomingTelemetry(MavEsp8266Interface* MavEsp8266Serial, FrSkyTxTelemetryInterface* FrSkyTxSerial);
    void readPackets();
    void udpWriteWithChecksum( uint8_t value, uint8_t* checksum );
    void udpWriteSwitchWithChecksum( PPMDecoder* ppmDecoder, uint8_t channelIndex, uint16_t minValue, uint8_t* checksum );
    void startVideo();

public:
    static ModeXiroMini instance;

    ModeXiroMini();

    void start();

    void loop(
        PPMDecoder* ppmDecoder,
        MavEsp8266Interface* MavEsp8266Serial,
        Smartport* sport, 
        FrSkyTxTelemetryInterface* FrSkyTxSerial
    );
};


