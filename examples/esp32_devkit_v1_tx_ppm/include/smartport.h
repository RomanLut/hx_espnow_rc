#pragma once

#include <Arduino.h>

#define SPORT_DIY_VALUES 3

//=====================================================================
//=====================================================================
class Smartport
{
private:
    unsigned long lastSend;
    uint8_t lastSensor;

    uint8_t RSSI;
    uint32_t A1,A2;

    uint8_t diy[SPORT_DIY_VALUES];

    unsigned short crc;

    HardwareSerial* serial;

    void sendByte(uint8_t byte);
    void sendCrc();
    void sendValue(uint16_t id, uint32_t value);
    void sendRSSI();
    void sendA1Voltage();
    void sendA2Value();
    void sendHeading();
    void sendDIYValue(int sensorId, int index);

public:

    Smartport( HardwareSerial* serial );

    void init();

    void loop();

    //value in dbm, 0..100
    void setRSSI( uint8_t value); 
    void setA1( uint32_t value); 
    void setA2( uint32_t value); 
    void setDIYValue( int index, uint8_t value);
};

