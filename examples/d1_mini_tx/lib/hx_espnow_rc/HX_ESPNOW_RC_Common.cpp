#include "HX_ESPNOW_RC_Common.h"

//=====================================================================
//=====================================================================
uint16_t HXRCGetChannelValueInt(const HXRCChannels& channels, uint8_t index )
{
    switch( index )
    {
        case 0:
            return channels.Ch1 + 1000;
        case 1:
            return channels.Ch2 + 1000;
        case 2:
            return channels.Ch3 + 1000;
        case 3:
            return channels.Ch4 + 1000;
        case 4:
            return channels.Ch5 + 1000;
        case 5:
            return channels.Ch6 + 1000;
        case 6:
            return channels.Ch7 + 1000;
        case 7:
            return channels.Ch8 + 1000;
        case 8:
            return channels.Ch9 + 1000;
        case 9:
            return channels.Ch10 + 1000;
        case 10:
            return channels.Ch11 + 1000;
        case 11:
            return channels.Ch12 + 1000;
        case 12:
            return channels.Ch13 + 1000;
        case 13:
            return channels.Ch14 + 1000;
        case 14:
            return channels.Ch15 + 1000;
        case 15:
            return channels.Ch16 + 1000;
        default:
            return 1000;
    }
}

//=====================================================================
//=====================================================================
void HXRCSetChannelValueInt(HXRCChannels& channels, uint8_t index, uint16_t data)
{
    data -= 1000;
    switch( index )
    {
        case 0:
            channels.Ch1 = data;
            break;
        case 1:
            channels.Ch2 = data;
            break;
        case 2:
            channels.Ch3 = data;
            break;
        case 3:
            channels.Ch4 = data;
            break;
        case 4:
            channels.Ch5 = data;
            break;
        case 5:
            channels.Ch6 = data;
            break;
        case 6:
            channels.Ch7 = data;
            break;
        case 7:
            channels.Ch8 = data;
            break;
        case 8:
            channels.Ch9 = data;
            break;
        case 9:
            channels.Ch10 = data;
            break;
        case 10:
            channels.Ch11 = data;
            break;
        case 11:
            channels.Ch12 = data;
            break;
        case 12:
            channels.Ch13 = data;
            break;
        case 13:
            channels.Ch14 = data;
            break;
        case 14:
            channels.Ch15 = data;
            break;
        case 15:
            channels.Ch16 = data;
            break;
    }
}
