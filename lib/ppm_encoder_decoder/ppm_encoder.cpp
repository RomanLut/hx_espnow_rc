#include "ppm_encoder.h"

#if defined(ESP8266)
#include <esp8266_peri.h>
#elif defined(ESP32)
#endif

//=====================================================================
//=====================================================================
PPMEncoder::PPMEncoder()
{
}

//=====================================================================
//=====================================================================
void PPMEncoder::init(uint8_t tx_pin, bool invert )
{
    lastPacket.init();
    lastPacket.failsafe = 1;
    lastPacketTime = millis();

#if defined(ESP8266)
//nothing to see here
#elif defined(ESP32)
//nothing to see here
#endif

}

//=====================================================================
//=====================================================================
void PPMEncoder::loop()
{
    static boolean state = true;
    static unsigned long next = ESP.getCycleCount();
    if (next >= ESP.getCycleCount()){
        if (state) {  //start pulse
            digitalWrite(PPM_SIG_PIN, PPM_INVERTED);
            next = next + (PPM_PULSE_LENGTH * PPM_CPU_MHZ);
            state = false;
    //        alivecount++;
        } else {  //end pulse and calculate when to start the next pulse
            static byte cur_chan_numb;
            static unsigned int calc_rest;

            digitalWrite(PPM_SIG_PIN, !PPM_INVERTED);
            state = true;

            if (cur_chan_numb >= PPM_CHANNEL_NUMBER) {
                cur_chan_numb = 0;
                calc_rest = calc_rest + PPM_PULSE_LENGTH;//
                next = next + ((PPM_FRAME_LENGTH - calc_rest) * PPM_CPU_MHZ);
                calc_rest = 0;
    //            digitalWrite(DEBUGPIN, !digitalRead(DEBUGPIN));
            } else {
                next = next + ((ppm[cur_chan_numb] - PPM_PULSE_LENGTH) * PPM_CPU_MHZ);
                calc_rest = calc_rest + ppm[cur_chan_numb];
                cur_chan_numb++;
            }
        }
    }
}

//=====================================================================
//=====================================================================
void PPMEncoder::setFailsafe( bool failsafe )
{
    this->lastPacket.failsafe = failsafe?1:0;
}

//=====================================================================
//=====================================================================
void PPMEncoder::setChannelValueDirect( uint8_t index, uint16_t value )
{
    this->lastPacket.setChannelValue( index, value );
}

//=====================================================================
//=====================================================================
//input value is in range 1000..2000
void PPMEncoder::setChannelValue( uint8_t index, uint16_t value ) 
{
    this->lastPacket.setChannelValue( index, constrain( map( value, 1000, 2000, PPM_MIN, PPM_MAX), 0, 2047) );
}

