#pragma once

#include <Arduino.h>

#include <stdio.h>
#include <stdarg.h>

#include "modeBase.h"

//=====================================================================
//=====================================================================
class ModeKYFPV : public ModeBase
{
private:
    unsigned long lastPacketTime;
    unsigned long retryConnectionTime;
    uint32_t packetsCount;
    uint32_t flipBeep;

    //States:
    //0 - inactive
    //1...22 - in effect
    //23...100 - wating to deactivate
    uint8_t takeOffCount;
    uint8_t landingCount;
    uint8_t stopCount;
    uint8_t GyroCount;

    WiFiUDP udpCMD;
    int state;

    boolean headless;

    void sendBeginPacket();
    void sendChannels(const HXChannels* channels);

    void fillOutgoingTelemetry(HC06Interface* externalBTSerial);
    void processIncomingTelemetry(HC06Interface* externalBTSerial);

    void processButton( int channelValue, uint8_t* state, const char* event);

public:
    static ModeKYFPV instance;
    static const char* name;

    ModeKYFPV();

    void start( JsonDocument* json );

    void loop(
        const HXChannels* channels,
        HC06Interface* externalBTSerial,
        Smartport* sport
    );
};


