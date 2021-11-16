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

    HXRCMaster hxrcMaster;
    HXRCSerialBuffer<512> hxrcTelemetrySerial;

    void setChannels(NKPPMDecoder* ppmDecoder);
    void fillOutgoingTelemetry(HC06Interface* externalBTSerial);
    void processIncomingTelemetry(HC06Interface* externalBTSerial);

public:
    static ModeEspNowRC instance;

    ModeEspNowRC();

    void start();

    void loop(
        NKPPMDecoder* ppmDecoder,
        HC06Interface* externalBTSerial,
        Smartport* sport
    );
};


