#pragma once

#include <Arduino.h>

//=====================================================================
//=====================================================================
class Smartport
{
private:
    unsigned long lastSend;
    uint8_t lastSensor;

    uint8_t RSSI;
    uint32_t A1,A2;

    unsigned short crc;

    HardwareSerial* serial;

    void sendByte(uint8_t byte);
    void sendCrc();
    void sendValue(uint16_t id, uint32_t value);
    void sendRSSI();
    void sendA1Voltage();
    void sendA2Value();
    void sendHeading();

public:

    Smartport( HardwareSerial* serial );

    void init();

    void loop();

    //value in dbm, 0..100
    void setRSSI( uint8_t value); 
    void setA1( uint32_t value); 
    void setA2( uint32_t value); 
};

