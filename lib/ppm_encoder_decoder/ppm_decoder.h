#pragma once
#if defined(ESP32) 

#include <Arduino.h>
#include <stdint.h>
#include "espRCPpm.h"
#include "ppm_packet.h"
//include for HXRCLOG usage
#include "HX_ESPNOW_RC_Common.h"
#define PPM_SYNC_FAILSAFE_MS            200
#define PPM_CHANNEL_NUMBER 16  //set the number of chanels
#define PPM_TIMER 0
#define PPM_PIN 14
#define PPM_NEUTRAL 1500 

//=====================================================================
//=====================================================================
class PPMDecoder
{
private:
    //40 - wait for packet footer( sync)
    //30..39 - skip byte
    //0 - expect header
    //1..23 - receive body
    //24 expect footer
    //25 expect header
    RCPpmIn _decoder = RCPpmIn((uint8_t) PPM_PIN,(uint8_t) PPM_TIMER,(uint8_t) PPM_CHANNEL_NUMBER, (uint16_t) PPM_NEUTRAL);
    uint8_t state;
    uint8_t syncCount;
    uint16_t packetsCount;
    uint8_t resyncCount;
    uint8_t resyncSkipCount;
    uint16_t failsafeCount;
    bool failsafeState;
    
    PPMPacket packet;
    PPMPacket lastPacket;
    unsigned long lastPacketTime;

    void parsePacket();
    void dumpPacket();
    void resync();
    void updateFailsafe();

public:
    PPMDecoder();

    void init( gpio_num_t gpio );

    uint16_t getChannelValue( uint8_t index );
    uint16_t getChannelValueInRange( uint8_t index, uint16_t from, uint16_t to );
    bool isOutOfSync();
    bool isFailsafe();

    void loop();

    void dump();
    static const int frequency = 1000000;  //1MHZ
    static const int max_pulses = 128;
    static const int idle_threshold = 3000;  //we require at least 3ms gap between frames
    bool read(uint32_t &width_high, uint32_t &width_low);
};



#endif