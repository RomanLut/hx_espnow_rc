#pragma once

#if defined(ESP8266)
#include <interrupts.h>
#elif defined (ESP32)
#include "freertos/ringbuf.h"
#endif

#include <Arduino.h>

//=====================================================================
//=====================================================================
class HXRCRingBufferInterface
{
    public:
        virtual bool send( const void* data, uint16_t lenToWrite ) = 0;
        virtual uint16_t receiveUpTo( uint16_t maxLen, uint8_t* toPtr ) = 0;
};


#if defined(ESP8266)

//=====================================================================
//=====================================================================
template<int Size> 
class HXRCRingBuffer : public HXRCRingBufferInterface
{
private:

    volatile uint8_t* pIn;
    volatile uint8_t* pOut;
    volatile uint8_t* pStart;
    volatile uint8_t* pEnd; 
    volatile uint16_t count;

    uint8_t buffer[Size];

    uint16_t getCount()
    {
        return this->count;
    }

    uint16_t getFreeCount()
    {
        return Size - this->count;
    }

    void insert(const uint8_t data)
    {
        *this->pIn = data;

        if (++this->pIn == this->pEnd)
        {
            this->pIn = this->pStart;
        }

        this->count++;
    }

    void insertBuffer(const uint8_t* pData, uint16_t length)
    {
        while ( length-- ) insert(*pData++);
    }

    uint8_t receive()
    {
        uint8_t data = *this->pOut;

        if (++this->pOut == this->pEnd)
        {
            this->pOut = this->pStart;
        }

        this->count--;

        return data;
    }

    // length should be less or equal to getCount()
    void receiveBuffer( uint8_t* buffer, uint16_t length )
    {
        while ( length-- ) *buffer++ = receive();
    }

    uint8_t peek()
    {
        return *this->pOut;
    }

public:
    HXRCRingBuffer() 
    {
        this->pIn     = buffer;
        this->pOut    = buffer;
        this->pStart  = &buffer[0];
        this->pEnd    = &buffer[Size];
        this->count  = 0;
    }

    bool hasData() const
    {
        return this->count != 0;
    }

    bool send( const void* data, uint16_t lenToWrite )
    {
        {     
            esp8266::InterruptLock lock;
            if ( getFreeCount() < lenToWrite ) return false;
            insertBuffer(reinterpret_cast<const uint8_t*>(data), lenToWrite);
        }
        return true;
    }

    //returns number of bytes received
    uint16_t receiveUpTo( uint16_t maxLen, uint8_t* toPtr )
    {
        {     
            esp8266::InterruptLock lock;

            if ( maxLen > this->count ) maxLen = this->count;
            if ( maxLen == 0 ) return 0;
            receiveBuffer(reinterpret_cast<uint8_t*>(toPtr), maxLen);
        }
        return maxLen;
    }


};

#elif defined (ESP32)

//=====================================================================
//=====================================================================
template<int Size> 
class HXRCRingBuffer : public HXRCRingBufferInterface
{
private:

    RingbufHandle_t buffferHandle;
    volatile bool bHasData;

public:
    HXRCRingBuffer() 
    {
        this->bHasData = false;
        this->buffferHandle = xRingbufferCreate(Size, RINGBUF_TYPE_BYTEBUF);
        if ( this->buffferHandle == NULL) 
        {
            Serial.print("HXRC: Failed create ring buffer");
        }    
    }

    //(may_ have data in buffer
    bool hasData() const
    {
        return this->bHasData;
    }

    ~HXRCRingBuffer() 
    {
        vRingbufferDelete(this->buffferHandle);
    }

    bool send( const void* data, uint16_t lenToWrite )
    {
        if ( lenToWrite == 0) return true;
        this->bHasData = true;
        return xRingbufferSend( this->buffferHandle, data, lenToWrite, (TickType_t)0 ) == pdTRUE;
    }

    //returns number of bytes received
    uint16_t receiveUpTo( uint16_t maxLen, uint8_t* toPtr )
    {
        uint16_t total = 0;
        while ( maxLen > 0 ) 
        {
            size_t returnedSize = 0;
            //two calls to xRingbufferReceiveUpTo() are required for retrieve data which wraps around
            void* ptr = xRingbufferReceiveUpTo( this->buffferHandle, &returnedSize, (TickType_t)0, maxLen );
            if ( ptr == NULL ) 
            {
                this->bHasData = false;
                break;
            }
            memcpy( toPtr, ptr, returnedSize );
            vRingbufferReturnItem( this->buffferHandle, ptr );
            toPtr += returnedSize;
            total += returnedSize;
            maxLen -= returnedSize;
        }
        return total;
    }


};

#endif
