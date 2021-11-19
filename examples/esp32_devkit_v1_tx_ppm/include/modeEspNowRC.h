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

    void setChannels(PPMDecoder* ppmDecoder);
    void fillOutgoingTelemetry(MavEsp8266Interface* MavEsp8266Serial);
    void processIncomingTelemetry(MavEsp8266Interface* MavEsp8266Serial, FrSkyTxTelemetryInterface* FrSkyTxSerial);

public:
    static ModeEspNowRC instance;

    ModeEspNowRC();

    void start();

    void loop(
        PPMDecoder* ppmDecoder,
        MavEsp8266Interface* MavEsp8266Serial,
        Smartport* sport,
        FrSkyTxTelemetryInterface* FrSkyTxSerial
    );
};


