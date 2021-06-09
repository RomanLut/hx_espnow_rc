#pragma once

#include <Arduino.h>

#define FAILSAFE_PACKETS_COUNT  20  //for 50ms packets period, 20 packets for failsafe period 1 second
#define PACKET_SEND_PERIOD_MS   50      //20Hz    
#define FAILSAFE_PERIOD_MS (PACKET_SEND_PERIOD_MS * FAILSAFE_PACKETS_COUNT)
#define FAILSAFE_PACKETS_BITMASK 0xfffff

typedef enum
{
    HXRCPKID_RCDATA     = 0,
    HXRCPKID_TELEMETRY  = 1
} HXRCPacketId;

#define HXRC_CHANNELS 16

#pragma pack (push)
#pragma pack (1)

typedef struct 
{
    uint16_t packetId   : 2;    //HXRCPacketId
    uint16_t sequenceId : 14;
} HXRCPayload;

typedef struct 
{
    uint16_t packetId   : 2;    //HXRCPacketId
    uint16_t sequenceId : 14;

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
    uint16_t Ch16       : 11;   //16*11 = 22 bytes, 24 bytes with id
} HXRCPayloadChannels;

#define HXRCPayloadSize_MAX 250
#define HXRCPayloadSize_MIN 2   //at least 2 bytes for packedId and sequenceId 

#define HXRCTelemetryHeaderSize ( 1 + 1 + 2 )
#define HXRCTelemetryDataSizeMAX ( HXRCPayloadSize_MAX - HXRCTelemetryHeaderSize )

typedef struct 
{
    uint16_t packetId   : 2;    //HXRCPacketId
    uint16_t sequenceId : 14;

    uint8_t rssi;  //transmitted from uav to ground

    uint8_t length;  //length of telemetry data, if any

    uint8_t data[HXRCTelemetryDataSizeMAX];

} HXRCPayloadTelemetry;

#pragma pack (pop)

//Ground transmitter state, uav telemetry transmitter state
typedef enum
{
    HXRCSS_READY_TO_SEND        = 0,
    HXRCSS_WAITING_CONFIRMATION = 1
} HXRCSenderStateEnum;

uint16_t inline HXRCGetChannelValueInt(HXRCPayloadChannels& channels, uint8_t index )
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

void inline HXRCSetChannelValueInt(HXRCPayloadChannels& channels, uint8_t index, uint16_t data)
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
