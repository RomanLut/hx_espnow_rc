#include "HX_ESPNOW_RC_Channels.h"

//=====================================================================
//=====================================================================
void HXRCChannels::init()
{
    ch1 = ch2 = ch3 = ch4 = ch5 = ch6 = ch7 = ch8 = ch9 = ch10 = ch11 = ch12 = ch13 = ch14 = ch15 = ch16 = 1000;
}

//=====================================================================
//=====================================================================
uint16_t HXRCChannels::getChannelValue( uint8_t index ) const
{
    switch( index )
    {
        case 0:
            return ch1;
        case 1:
            return ch2;
        case 2:
            return ch3;
        case 3:
            return ch4;
        case 4:
            return ch5;
        case 5:
            return ch6;
        case 6:
            return ch7;
        case 7:
            return ch8;
        case 8:
            return ch9;
        case 9:
            return ch10;
        case 10:
            return ch11;
        case 11:
            return ch12;
        case 12:
            return ch13;
        case 13:
            return ch14;
        case 14:
            return ch15;
        case 15:
            return ch16;
        default:
            return 1000;
    }
}

//=====================================================================
//=====================================================================
void HXRCChannels::setChannelValue( uint8_t index, uint16_t data )
{
    switch( index )
    {
        case 0:
            ch1 = data;
            break;
        case 1:
            ch2 = data;
            break;
        case 2:
            ch3 = data;
            break;
        case 3:
            ch4 = data;
            break;
        case 4:
            ch5 = data;
            break;
        case 5:
            ch6 = data;
            break;
        case 6:
            ch7 = data;
            break;
        case 7:
            ch8 = data;
            break;
        case 8:
            ch9 = data;
            break;
        case 9:
            ch10 = data;
            break;
        case 10:
            ch11 = data;
            break;
        case 11:
            ch12 = data;
            break;
        case 12:
            ch13 = data;
            break;
        case 13:
            ch14 = data;
            break;
        case 14:
            ch15 = data;
            break;
        case 15:
            ch16 = data;
            break;
    }
}
