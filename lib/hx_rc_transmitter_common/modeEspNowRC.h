#pragma once

#include <Arduino.h>

#include <stdio.h>
#include <stdarg.h>

#include "HX_ESPNOW_RC_Master.h"
#include "HX_ESPNOW_RC_SerialBuffer.h"

#include "modeBase.h"

//=====================================================================
//=====================================================================
class ModeEspNowRC : public ModeBase
{
private:
    unsigned long lastStats;
    bool lastFailsafe;

    HXRCMaster hxrcMaster;
    HXRCSerialBuffer<512> hxrcTelemetrySerial;

    void setChannels(const HXChannels* channels);
    void fillOutgoingTelemetry(HC06Interface* externalBTSerial);
    void processIncomingTelemetry(HC06Interface* externalBTSerial);

public:
    static ModeEspNowRC instance;
    static const char* name;

    bool LRMode;

    ModeEspNowRC();

    void start( JsonDocument* json );

    void loop(
        const HXChannels* channels,
        HC06Interface* externalBTSerial,
        Smartport* sport
    );
};


