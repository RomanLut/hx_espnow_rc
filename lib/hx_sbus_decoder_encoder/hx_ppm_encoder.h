#pragma once

/*
Usage: 

init:
    hxPPMEncoder.init( channelsCount, pin );

 loop:
    hxPPMEncoder.setfailsafe(flag);
    hxPPMEncoder.setChannelValue( index, value );  //0..6, 1000...2000
    hxPPMEncoder.commit();  //atomic comit of all channels and failsafe flag
  
*/

#include <Arduino.h>
#include <stdint.h>

//=====================================================================
//=====================================================================
class HXPPMEncoder
{
private:
public:
    HXPPMEncoder();

    //channelscount == 4...16
    void init( uint8_t _channelsCount, uint8_t _tx_pin );

    void setFailsafe( bool _failsafe );
    void setChannelValue( uint8_t index, uint16_t value );
    void commit();
};



