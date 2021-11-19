#pragma once
#include "modeBase.h"
#include "tx_config.h"

//=====================================================================
//=====================================================================
class ModeIdle : public ModeBase
{
private:
public:
    static ModeIdle instance;

    void start();

    void loop(
        PPMDecoder* ppmDecoder,
        MavEsp8266Interface* MavEsp8266Serial,
        Smartport* sport,
        FrSkyTxTelemetryInterface* FrSkyTxSerial
    );
};


