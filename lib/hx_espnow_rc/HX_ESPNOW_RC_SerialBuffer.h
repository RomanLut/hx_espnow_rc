#pragma once

#include "HX_ESPNOW_RC_Base.h"

//=====================================================================
//=====================================================================
//It is unefficent to read/write RingBuffer byte by byte
//This class is an adapter around RingBuffer with Serial-like interface
//and flush() method.
//flush() should be called in a loop to fill serial buffer from RingBuffer,
//and send filled buffer to RingBuffer
template<int Size> 
class HXRCSerialBuffer
{
private:

    uint8_t inBuffer[Size];
    int inCount;
    int inHead;

    uint8_t outBuffer[Size];
    int outCount;
    int outHead;

    HXRCBase* base;

public:
    HXRCSerialBuffer( HXRCBase* base ) 
    {
        this->base = base;

        this->inCount = 0;
        this->inHead = 0;

        this->outCount = 0;
        this->outHead = 0;
    }

    uint16_t getAvailable() 
    {
        if ( this->inCount == 0 ) flushIn();
        return this->inCount;
    }
    
    uint16_t getAvailableForWrite() 
    {
        if ( Size ==  this->outCount ) flushOut();
        return Size - this->outCount;
    }

    uint8_t read()
    {
        if ( this->inCount == 0) return 0;
        uint8_t res = inBuffer[inHead];
        if ( ++inHead == Size) inHead = 0;
        inCount--;
        return res;
    }

/*
    uint8_t peek()
    {
        if ( this->inCount == 0) return 0;
        return inBuffer[inHead];
    }
*/
    void write(uint8_t c)
    {
        if ( outCount == Size ) flushOut();
        if ( outCount == Size ) return;
        outBuffer[outHead] = c;
        if ( ++outHead == Size ) outHead = 0;
        outCount++;
    }

    void flushOut()
    {
        // 1)
        //--------DDDDDDDD-----------
        //        |       h
        //        h-c                  
        // 2)
        //DDDDD-----------------DDD
        //     h                |  
        //                      h-c+SIZE
        while ( outCount > 0 )
        {
            int p = outHead - outCount;
            if ( p < 0 ) p += Size;
            int countEnd = Size - p;
            int c = outCount < countEnd ? outCount : countEnd;
            if ( this->base->sendOutgoingTelemetry( &outBuffer[p], c) )
            {
                outCount -= c;
            }
            else
            {
                break;
            }
        }
    }

    void flushIn()
    {
        //1)
        //---------DDDDDDDD------
        //         h       |
        //                 h+c
        //2)
        //DDDDDDDD-------DDDDDDDD
        //        |      h       
        //        h+c-Size         

        while( inCount < Size )
        {
            int p = inHead + inCount;
            if ( p >= Size ) p -= Size;
            int freeCount = Size - inCount;

            int countEnd = Size - p;
            int c = freeCount < countEnd ? freeCount : countEnd;

            int a = this->base->getIncomingTelemetry( c, &inBuffer[p]);
            if ( a == 0 ) break;
            inCount += a;
        }
    }

    void flush()
    {
        flushOut();
        flushIn();
    }


};

