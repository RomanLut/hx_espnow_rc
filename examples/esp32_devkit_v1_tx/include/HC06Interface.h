#pragma once

#include <Arduino.h>
#include <SoftwareSerial.h>


//=====================================================================
//=====================================================================
class HC06Interface 
{
private:
    SoftwareSerial serial;

    void flushResponse();
public:

    HC06Interface();

    void init();

    int available();
    int read();
    int availableForWrite();
    size_t write(uint8_t c);
};

