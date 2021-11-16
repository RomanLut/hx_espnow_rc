#include "nk_ppm_decoder.h"

#define SYNC_COUNT  5
//=====================================================================
//=====================================================================
NKPPMDecoder::NKPPMDecoder()
{
}

//=====================================================================
//=====================================================================
void NKPPMDecoder::init(gpio_num_t gpio )
{
    lastPacket.init();
    lastPacket.failsafe = 1;
    lastPacketTime = millis();

#if defined(ESP8266)
    //Serial1.begin(100000, SERIAL_8E2, SerialMode::SERIAL_RX_ONLY, gpio, false );  
#elif defined (ESP32)
    //Serial1.begin(100000, SERIAL_8E2, gpio, -1, false );  
#endif

    //pinMode(gpio,INPUT);

    state = 40;
    syncCount = 0;
    packetsCount = 0;
    resyncSkipCount = 0;
    resyncCount = 0;
    failsafeCount = 0;
    failsafeState = false;

    rmt_config_t config;
    config.rmt_mode = RMT_MODE_RX;
    config.channel = RMT_CHANNEL_0;
    config.clk_div = 80;   //80MHZ APB clock to the 1MHZ target frequency
    config.gpio_num = gpio;
    config.mem_block_num = 2; //each block could store 64 pulses
    config.rx_config.filter_en = true;
    config.rx_config.filter_ticks_thresh = 8;
    config.rx_config.idle_threshold = idle_threshold;

    rmt_config(&config);
    rmt_driver_install(config.channel, max_pulses * 8, 0);
    rmt_get_ringbuf_handle(config.channel, &handle);
    rmt_rx_start(config.channel, true);
}

//=====================================================================
//=====================================================================
void NKPPMDecoder::loop()
{


    updateFailsafe();
}

//=====================================================================
//=====================================================================
void NKPPMDecoder::parsePacket()
{
    this->lastPacket = packet;
    this->lastPacketTime = millis();

          //dump();
          //delay(200);

}

//=====================================================================
//=====================================================================
uint16_t NKPPMDecoder::getChannelValue( uint8_t index ) const
{
    return this->lastPacket.getChannelValue( index );
}

//=====================================================================
//=====================================================================
bool NKPPMDecoder::isOutOfSync() const
{
    return (this->syncCount < SYNC_COUNT) || this->lastPacket.failsafe;
}

//=====================================================================
//=====================================================================
bool NKPPMDecoder::isFailsafe() const
{
    return this->failsafeState;
}

//=====================================================================
//=====================================================================
void NKPPMDecoder::updateFailsafe()
{
    bool res = this->lastPacket.failsafe;

    unsigned long t = millis();
    unsigned long deltaT = t - this->lastPacketTime;

    if ( deltaT >=  PPM_SYNC_FAILSAFE_MS )
    {
        this->lastPacketTime = t - PPM_SYNC_FAILSAFE_MS;
        res = true;
    }

    if ( !this->failsafeState && res )
    {
        this->failsafeCount++;
    }

    this->failsafeState = res;
}


//=====================================================================
//=====================================================================
void NKPPMDecoder::dump() const
{
    Serial.print("Failsafe: ");
    Serial.print(this->isFailsafe()?1: 0);
    Serial.print(" (");
    Serial.print(this->failsafeCount);
    Serial.println(")");

    Serial.print("OutOfSync:");
    Serial.println(this->isOutOfSync()?1: 0);

    Serial.print("PacketsCount: ");
    Serial.print(this->packetsCount);
    Serial.print("  ResyncCount: ");
    Serial.println(this->resyncCount);

    for ( int i = 0; i < 16; i++ ) 
    {
        Serial.print("Channel");
        Serial.print(i);
        Serial.print(": ");
        Serial.println(this->getChannelValue(i));
    }

}

//=====================================================================
//=====================================================================
void NKPPMDecoder::dumpPacket() const
{
    uint8_t* p = ((uint8_t*)&packet);
    for ( int i = 0; i < PPM_PACKET_SIZE; i++ ) 
    {
        Serial.print(*p++, HEX);
        Serial.print( " ");
    }
    Serial.println("");
}


//=====================================================================
//=====================================================================
void NKPPMDecoder::resync()
{
    resyncSkipCount++;
    if ( resyncSkipCount > 10 ) resyncSkipCount  = 0;
    state = 40 - resyncSkipCount;
    resyncCount++;
}

//=====================================================================
//=====================================================================
uint16_t NKPPMDecoder::getChannelValueInRange( uint8_t index, uint16_t from, uint16_t to ) const  
{
    return map( constrain( this->getChannelValue(index), PPM_MIN, PPM_MAX ), PPM_MIN, PPM_MAX, from, to );
}