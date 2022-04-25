#pragma once

#include <Arduino.h>


//=====================================================================
//=====================================================================
class HC06Interface 
{
private:
    HardwareSerial* serial;
    void flushResponse();
public:

    HC06Interface();

    void init(HardwareSerial* serial, int rx_pin, int tx_pin);

    int available();
    int read();
    int availableForWrite();
    size_t write(uint8_t c);
};

