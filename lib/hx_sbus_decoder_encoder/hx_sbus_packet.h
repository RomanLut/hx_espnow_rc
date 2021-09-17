#pragma once

#include <Arduino.h>
#include <stdint.h>

#define SBUS_PACKET_SIZE    ( 1 + 22 + 1 + 1)
#define SBUS_HEADER         0x0f
#define SBUS_FOOTER         0x00

#define SBUS_MIN            173
#define SBUS_DID            992
#define SBUS_MAX            1811


#pragma pack (push)
#pragma pack (1)

//https://os.mbed.com/users/Digixx/notebook/futaba-s-bus-controlled-by-mbed/
//=====================================================================
//=====================================================================
typedef struct 
{
    uint8_t header;
    
    uint16_t ch1 : 11; //SBUS_MIN...SBUS_MAX
    uint16_t ch2 : 11;
    uint16_t ch3 : 11;
    uint16_t ch4 : 11;
    uint16_t ch5 : 11;
    uint16_t ch6 : 11;
    uint16_t ch7 : 11;
    uint16_t ch8 : 11;
    uint16_t ch9 : 11;
    uint16_t ch10 : 11;
    uint16_t ch11 : 11;
    uint16_t ch12 : 11;
    uint16_t ch13 : 11;
    uint16_t ch14 : 11;
    uint16_t ch15 : 11;
    uint16_t ch16 : 11;

    //bit field filling is implementation dependent.
    //ESP8266: ch17 is bit 7
    uint8_t ch17 : 1;
    uint8_t ch18 : 1;
    uint8_t frameLost : 1;
    uint8_t failsafe: 1;
    uint8_t reserved: 4;

    uint8_t footer;

    void init();
    uint16_t getChannelValue( uint8_t index ) const;
    void setChannelValue( uint8_t index, uint16_t data );
} HXSBUSPacket;

#pragma pack (pop)

