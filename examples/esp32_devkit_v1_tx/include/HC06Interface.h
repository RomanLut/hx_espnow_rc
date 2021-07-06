#pragma once

#include <Arduino.h>
#include <SoftwareSerial.h>


//=====================================================================
//=====================================================================
class HC06Interface 
{
private:
    HardwareSerial serial;
    void flushResponse();
public:

    HC06Interface( int UART);

    void init();

    int available();
    int read();
    int availableForWrite();
    size_t write(uint8_t c);
};

