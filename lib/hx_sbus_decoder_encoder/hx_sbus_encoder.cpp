#include "hx_sbus_encoder.h"

//=====================================================================
//=====================================================================
HXSBUSEncoder::HXSBUSEncoder()
{
}

//=====================================================================
//=====================================================================
void HXSBUSEncoder::init( HardwareSerial& serial, uint8_t tx_pin, bool invert )
{
    lastPacket.init();
    lastPacket.failsafe = 1;
    lastPacketTime = millis();

    serial.begin(100000, SERIAL_8E2, SerialMode::SERIAL_TX_ONLY, tx_pin, invert );  
    pinMode(tx_pin,OUTPUT);
}

//=====================================================================
//=====================================================================
void HXSBUSEncoder::loop( HardwareSerial& serial )
{
    if (Serial1.availableForWrite() < sizeof( this->lastPacket )) return;

    unsigned long t = millis();
    if ( (t - this->lastPacketTime)  < SBUS_PERIOD_MS ) return;

    this->lastPacketTime = t;

    serial.write( (const uint8_t*)&this->lastPacket, sizeof ( this->lastPacket ));
}

//=====================================================================
//=====================================================================
void HXSBUSEncoder::setFailsafe( bool failsafe )
{
    this->lastPacket.failsafe = failsafe;
}

//=====================================================================
//=====================================================================
void HXSBUSEncoder::setChannelValue( uint8_t index, uint16_t value )
{
    this->lastPacket.setChannelValue( index, value );
}


