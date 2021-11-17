#include "ppm_decoder.h"

#define SYNC_COUNT  5
//=====================================================================
//=====================================================================
PPMDecoder::PPMDecoder()
{
}

//=====================================================================
//=====================================================================
void PPMDecoder::init(gpio_num_t gpio )
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
}

//=====================================================================
//=====================================================================
void PPMDecoder::loop()
{
    if (this->_decoder.update()){

        for(uint8_t i = 0; i < PPM_CHANNEL_NUMBER; ++i){
            HXRCLOG.printf("rc:\t%i\t", this->_decoder.getChannel(i));
            packet.setChannelValue(i,this->_decoder.getChannel(i)) ;
        }
        HXRCLOG.println();
        packet.failsafe = 1 ;
        packet.frameLost = 0 ;
        parsePacket();
    }
    
    updateFailsafe();
}

//=====================================================================
//=====================================================================
void PPMDecoder::parsePacket()
{
    this->lastPacket = packet;
    this->lastPacketTime = millis();

          //dump();
          //delay(200);

}

//=====================================================================
//=====================================================================
uint16_t PPMDecoder::getChannelValue( uint8_t index )
{
    return this->_decoder.getChannel(index) ;
}

//=====================================================================
//=====================================================================
bool PPMDecoder::isOutOfSync() 
{
    return (this->syncCount < SYNC_COUNT) || this->lastPacket.failsafe;
}

//=====================================================================
//=====================================================================
bool PPMDecoder::isFailsafe() 
{
    return this->failsafeState;
}

//=====================================================================
//=====================================================================
void PPMDecoder::updateFailsafe()
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
void PPMDecoder::dump()
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
void PPMDecoder::dumpPacket() 
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
void PPMDecoder::resync()
{
    resyncSkipCount++;
    if ( resyncSkipCount > 10 ) resyncSkipCount  = 0;
    state = 40 - resyncSkipCount;
    resyncCount++;
}

//=====================================================================
//=====================================================================
uint16_t PPMDecoder::getChannelValueInRange( uint8_t index, uint16_t from, uint16_t to )
{
    return map( constrain( this->getChannelValue(index), PPM_MIN, PPM_MAX ), PPM_MIN, PPM_MAX, from, to );
}