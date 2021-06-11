#pragma once

#include <Arduino.h>

#define PACKET_SEND_PERIOD_MS   100      //10Hz    
#define FAILSAFE_PERIOD_MS      1000

#define HXRC_CHANNELS 16

#pragma pack (push)
#pragma pack (1)

#define HXRC_PAYLOAD_SIZE_MAX 250

#define HXRC_TRANSMITTER_PAYLOAD_SIZE_BASE (2 + 22 + 1 )  //sequenceId, channels, telemetry length
//#define HXRC_TRANSMITTER_TELEMETRY_SIZE_MAX ( HXRC_PAYLOAD_SIZE_MAX - HXRC_TRANSMITTER_PAYLOAD_SIZE_BASE )
#define HXRC_TRANSMITTER_TELEMETRY_SIZE_MAX 64  //limit packet size  to achieve target packet rate

typedef struct 
{
    uint16_t Ch1        : 11; 
    uint16_t Ch2        : 11;
    uint16_t Ch3        : 11;
    uint16_t Ch4        : 11;
    uint16_t Ch5        : 11;
    uint16_t Ch6        : 11;
    uint16_t Ch7        : 11;
    uint16_t Ch8        : 11;
    uint16_t Ch9        : 11;
    uint16_t Ch10       : 11;
    uint16_t Ch11       : 11;
    uint16_t Ch12       : 11;
    uint16_t Ch13       : 11;
    uint16_t Ch14       : 11;
    uint16_t Ch15       : 11;
    uint16_t Ch16       : 11;   //16*11 = 22 bytes
} HXRCChannels;


typedef struct 
{
    //sequenceId increments with each sent packet
    //and each packet which was not sent in time ( transmitter is required to keep PACKET_SEND_PERIOD_MS miminum rate)
    uint16_t sequenceId;

    HXRCChannels channels;

    uint8_t length;
    uint8_t data[HXRC_TRANSMITTER_TELEMETRY_SIZE_MAX];
} HXRCPayloadTransmitter;

#define HXRC_RECEIVER_PAYLOAD_SIZE_BASE (2 + 1 + 1 )  //sequenceId, rssi, telemetry length
#define HXRC_RECEIVER_TELEMETRY_SIZE_MAX ( HXRC_PAYLOAD_SIZE_MAX - HXRC_TRANSMITTER_PAYLOAD_SIZE_BASE )

typedef struct 
{
    uint16_t sequenceId;

    int8_t rssi;

    uint8_t length;
    uint8_t data[HXRC_RECEIVER_TELEMETRY_SIZE_MAX];
} HXRCPayloadReceiver;

#pragma pack (pop)

typedef enum
{
    HXRCSS_READY_TO_SEND        = 0,
    HXRCSS_WAITING_CONFIRMATION = 1
} HXRCSenderStateEnum;

uint16_t inline HXRCGetChannelValueInt(HXRCChannels& channels, uint8_t index )
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

void inline HXRCSetChannelValueInt(HXRCChannels& channels, uint8_t index, uint16_t data)
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
