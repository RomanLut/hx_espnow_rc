#pragma once

#include <Arduino.h>

//=====================================================================
//=====================================================================
class Smartport
{
private:
    unsigned long lastSend;
    unsigned long lastSensor;

    uint8_t RSSI;

    unsigned short crc;

    HardwareSerial* serial;

    void sendByte(uint8_t byte);
    void sendCrc();
    void sendValue(uint16_t id, uint32_t value);
    void sendRSSI();
    void sendA2voltage();
    void sendHeading();

public:

    Smartport( HardwareSerial* serial );

    void init();

    void loop();

    //value in dbm, 0..100
    void setRSSI( uint8_t value); 
};

