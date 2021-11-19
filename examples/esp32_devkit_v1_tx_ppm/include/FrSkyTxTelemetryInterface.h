#pragma once

#include <Arduino.h>
#include <HardwareSerial.h>
#include "tx_config.h"


//=====================================================================
//=====================================================================
class FrSkyTxTelemetryInterface 
{
private:
    HardwareSerial* serial;
    void flushResponse();
public:

    FrSkyTxTelemetryInterface( HardwareSerial* serial );

    void init();

    int available();
    int read();
    int availableForWrite();
    size_t write(uint8_t c);
};

