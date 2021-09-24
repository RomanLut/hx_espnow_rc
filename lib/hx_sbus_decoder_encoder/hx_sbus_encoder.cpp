#include "hx_sbus_encoder.h"

#if defined(ESP8266)
#include <esp8266_peri.h>
#elif defined(ESP32)
#endif

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

#if defined(ESP8266)
    //FIXME: Arduino library for ESP8266 does not contain code to invert Serial1.
    //Sneak flags to SerialConfig
    int serialConfig = SERIAL_8E2;
    if  (!invert )
    {
        serialConfig |= BIT(UCTXI);
    }
    serial.begin(100000, (SerialConfig)serialConfig, SerialMode::SERIAL_TX_ONLY, tx_pin, !invert );  
#elif defined(ESP32)
    serial.begin(100000, SERIAL_8E2, -1, tx_pin, !invert );  
#endif

}

//=====================================================================
//=====================================================================
void HXSBUSEncoder::loop( HardwareSerial& serial )
{
    if (serial.availableForWrite() < sizeof( HXSBUSPacket )) return;

    unsigned long t = millis();
    if ( (t - this->lastPacketTime)  < SBUS_RATE_MS ) return;

    this->lastPacketTime = t;

    serial.write( (const uint8_t*)&this->lastPacket, sizeof ( HXSBUSPacket ));
}

//=====================================================================
//=====================================================================
void HXSBUSEncoder::setFailsafe( bool failsafe )
{
    this->lastPacket.failsafe = failsafe?1:0;
}

//=====================================================================
//=====================================================================
void HXSBUSEncoder::setChannelValueDirect( uint8_t index, uint16_t value )
{
    this->lastPacket.setChannelValue( index, value );
}

//=====================================================================
//=====================================================================
//input value is in range 1000..2000
void HXSBUSEncoder::setChannelValue( uint8_t index, uint16_t value ) 
{
    this->lastPacket.setChannelValue( index, constrain( map( value, 1000, 2000, SBUS_MIN, SBUS_MAX), 0, 2047) );
}

