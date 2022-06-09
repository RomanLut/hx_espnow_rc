#include "hx_mavlink_rc_encoder.h"

#include <common/mavlink.h>

#if defined(ESP8266)
#include <esp8266_peri.h>
#elif defined(ESP32)
#endif

//=====================================================================
//=====================================================================
HXMavlinkRCEncoder::HXMavlinkRCEncoder()
{
}

//=====================================================================
//=====================================================================
void HXMavlinkRCEncoder::initChannels()
{
    for ( int i = 0; i < MAVLINK_RC_CHANNELS_COUNT; i++ )
    {
        this->channels[i] = 1000;
    }
}

//=====================================================================
//=====================================================================
void HXMavlinkRCEncoder::init( uint16_t packetRateMS, bool mavlink_v1 )
{
    this->mavlink_v1 = mavlink_v1;
    this->packetRateMS = packetRateMS;

    this->initChannels();
    
    this->lastPacketTime = millis();
    this->failsafe = true;
}

//=====================================================================
//=====================================================================
void HXMavlinkRCEncoder::loop( HardwareSerial& serial )
{
    if (serial.availableForWrite() < 34 ) return;

    unsigned long t = millis();
    if ( (t - this->lastPacketTime)  < this->packetRateMS ) return;

    if ( this->failsafe ) return;

    mavlink_message_t msg;
    mavlink_msg_rc_channels_override_pack( 
            1 , MAV_COMP_ID_USER1, 
            &msg,
            1, MAV_COMP_ID_AUTOPILOT1,
            this->channels[0], this->channels[1], this->channels[2], this->channels[3],
            this->channels[4], this->channels[5], this->channels[6], this->channels[7],
            this->channels[8], this->channels[9], this->channels[10], this->channels[11],
            this->channels[12], this->channels[13], this->channels[14], this->channels[15],
            0, 0
         );

    if ( this->mavlink_v1 )
    {
        //pack MAVLINK_STATUS_FLAG_OUT_MAVLINK1 flag and recalculate CRC
        mavlink_get_channel_status(MAVLINK_COMM_0)->flags = MAVLINK_STATUS_FLAG_OUT_MAVLINK1;

        mavlink_finalize_message_chan(
            &msg, 1, MAV_COMP_ID_USER1,
            MAVLINK_COMM_0, 
            MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE_MIN_LEN, MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE_LEN, MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE_CRC);
    }

    uint8_t sbuf[MAVLINK_MAX_PACKET_LEN];
    int len = mavlink_msg_to_send_buffer(sbuf, &msg); //len = 18(v1) or 34(v2)  

    if (serial.availableForWrite() < len ) return;

    serial.write( sbuf, len );

    this->lastPacketTime = t;
}

//=====================================================================
//=====================================================================
void HXMavlinkRCEncoder::setFailsafe( bool failsafe )
{
    this->failsafe = failsafe;
}

//=====================================================================
//=====================================================================
//input value is in range 1000..2000
void HXMavlinkRCEncoder::setChannelValue( uint8_t index, uint16_t value ) 
{
    if ( index < MAVLINK_RC_CHANNELS_COUNT )
    {
        this->channels[index] = value;
    }
}

