#pragma once

#include <Arduino.h>
#include <HardwareSerial.h>
#include "tx_config.h"

//=====================================================================
//=====================================================================
class MavEsp8266Interface 
{
private:
    HardwareSerial* serial;
    void flushResponse();
public:

    MavEsp8266Interface( HardwareSerial* serial );

    void init();

    int available();
    int read();
    int availableForWrite();
    size_t write(uint8_t c);
};

