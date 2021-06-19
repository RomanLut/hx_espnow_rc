#pragma once

#include <Arduino.h>
#include <stdint.h>

#pragma pack (push)
#pragma pack (1)

//=====================================================================
//=====================================================================
typedef struct 
{
    uint16_t ch1        : 11; 
    uint16_t ch2        : 11;
    uint16_t ch3        : 11;
    uint16_t ch4        : 11;
    uint16_t ch5        : 11;
    uint16_t ch6        : 11;
    uint16_t ch7        : 11;
    uint16_t ch8        : 11;
    uint16_t ch9        : 11;
    uint16_t ch10       : 11;
    uint16_t ch11       : 11;
    uint16_t ch12       : 11;
    uint16_t ch13       : 11;
    uint16_t ch14       : 11;
    uint16_t ch15       : 11;
    uint16_t ch16       : 11;   //16*11 = 22 bytes

    void init();
    uint16_t getChannelValue( uint8_t index ) const;
    void setChannelValue(uint8_t index, uint16_t data);
} HXRCChannels;

#pragma pack (pop)
