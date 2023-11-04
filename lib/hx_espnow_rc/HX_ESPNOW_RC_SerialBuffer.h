#pragma once

#include "HX_ESPNOW_RC_Base.h"

//=====================================================================
//=====================================================================
class HXRCSerialBufferBase
{
    public:
        virtual uint16_t getAvailableForWrite() = 0;
        virtual void write(uint8_t c) = 0;

        void writeBuffer(const uint8_t* buf, int size) 
        {
            while (size > 0)
            {
                this->write(*buf++);
                size--;
            }
        }
};


//=====================================================================
//=====================================================================
//It is unefficent to read/write RingBuffer byte by byte
//This class is an adapter around RingBuffer with Serial-like interface
//and flush() method.
//flush() should be called in a loop to fill serial buffer from RingBuffer,
//and send filled buffer to RingBuffer
template<int Size> 
class HXRCSerialBuffer : public HXRCSerialBufferBase
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
    
    virtual uint16_t getAvailableForWrite()  override
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


    //returns number of bytes actually read
    int readUpTo(uint8_t* buffer, int maxCount)
    {
        int res = this->inCount;
        if ( res == 0 ) return 0;
        
        int toTail = Size - inHead;
        if ( toTail < res ) res = toTail;
        if (res > maxCount ) res = maxCount;

        memcpy( buffer, &(inBuffer[inHead]), res);
        inHead += res;

        if ( inHead == Size) inHead = 0;
        inCount -= res;

        return res;
    }

    virtual void write(uint8_t c) override
    {
        if ( outCount == Size ) flushOut();
        if ( outCount == Size ) return;
        outBuffer[outHead] = c;
        if ( ++outHead == Size ) outHead = 0;
        outCount++;
    }

    int flushOut()
    {
        int cout = 0;
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
            //if ( c > HXRC_TELEMETRY_BUFFER_SIZE) c = HXRC_TELEMETRY_BUFFER_SIZE;
            int max = this->base->getOutgoingTelemetryBufferAvailableForWrite();  
            if ( c > max ) c = max;
            if ( c == 0) break;
            if ( this->base->sendOutgoingTelemetry( &outBuffer[p], c) )
            {
                outCount -= c;
                cout += c;
            }
            else
            {
                break;
            }
        }
        return cout;
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

