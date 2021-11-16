#include "nk_ppm_decoder.h"

#define SYNC_COUNT  5
//=====================================================================
//=====================================================================
NKPPMDecoder::NKPPMDecoder()
{
}

//=====================================================================
//=====================================================================
void NKPPMDecoder::init(int gpio )
{
    lastPacket.init();
    lastPacket.failsafe = 1;
    lastPacketTime = millis();

#if defined(ESP8266)
    Serial1.begin(100000, SERIAL_8E2, SerialMode::SERIAL_RX_ONLY, gpio, false );  
#elif defined (ESP32)
    Serial1.begin(100000, SERIAL_8E2, gpio, -1, false );  
#endif

    pinMode(gpio,INPUT);

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
void NKPPMDecoder::loop()
{
    bool reuse = false;
    uint8_t incomingByte;
    while (Serial1.available() > 0 || reuse) 
    {
        if ( !reuse ) incomingByte = Serial1.read();
        reuse = false;

        uint8_t* pNext = ((uint8_t*)&packet) + state;

        if ( state == 40 )  //sync to the packet end
        {
            if ( incomingByte == PPM_FOOTER )
            {
                state = 0;
                syncCount = 0;
            }
        }
        else if ( state >= 30 )  //skip bytes for resync
        {
            state++;
        } 
        else if ( state == 0 )  //expect packet start
        {
            if ( incomingByte == PPM_HEADER )
            {
                /*
                for ( int i = 0; i < PPM_PACKET_SIZE; i++ ) 
                {
                    Serial.print(Serial1.read(), HEX);
                    Serial.print( " ");
                }
                Serial.println("");

                delay(200);
                */


                *pNext = incomingByte;
                state++;



            }
            else
            {
                reuse = true;
                resync();
            }
        }
        else if ( state == (PPM_PACKET_SIZE-1)  )  //expect footer
        {
            if ( incomingByte == PPM_FOOTER )
            {
                *pNext = incomingByte;
                state++;
            }
            else
            {
                reuse = true;
                resync();
            }
        }
        else if ( state == PPM_PACKET_SIZE  )  //check if next packet starts as expected
        {
            if ( incomingByte == PPM_HEADER ) //next packet started as expected
            {
                packetsCount++;
                if ( syncCount == SYNC_COUNT ) //get good sync ( at least 5 packets) before using data
                {
                    parsePacket();
                    state = 1;  
                }
                else
                {
                    syncCount++;
                    state = 1;
                }
            }
            else
            {
                resync();
            }

        }
        else
        {
            //receive packet body
            *pNext = incomingByte;
            state++;
        }
    }

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