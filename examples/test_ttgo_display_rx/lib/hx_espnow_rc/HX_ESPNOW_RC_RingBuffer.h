#pragma once

#include <Arduino.h>

//=====================================================================
//=====================================================================
template<int Size> class HXRCRingBufer
{
private:
    uint8_t* pIn;
    uint8_t* pOut;
    uint8_t* pStart;
    uint8_t* pEnd; 
    uint16_t count;

    uint8_t buffer[Size];

public:
    HXRCRingBufer() 
    {
        this->pIn     = buffer;
        this->pOut    = buffer;
        this->pStart  = &buffer[0];
        this->pEnd    = &buffer[Size];
        this->count  = 0;
    }

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

    uint8_t remove()
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
    void peekToBuffer( uint8_t* buffer, uint16_t length )
    {
        uint8_t* pData = this->pOut;

        while ( length-- ) 
        {
            *buffer++ = *pData;
            if (++pData == this->pEnd)
            {
                pData = this->pStart;
            }
        }
    }

    uint8_t peek()
    {
        return *this->pOut;
    }
};
