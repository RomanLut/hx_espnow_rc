#pragma once

#include <Arduino.h>
#include <stdint.h>

#include <SD_MMC.h>

//=====================================================================
//=====================================================================
template<int Size> 
class HXFileLog
{
private:
    uint8_t outBuffer[Size];
    int outCount;
    int outHead;

    uint16_t getAvailableForWrite()
    {
        return Size - this->outCount;
    }

    void write(uint8_t c)
    {
        if ( outCount == Size ) flush();
        if ( outCount == Size ) return;
        outBuffer[outHead] = c;
        if ( ++outHead == Size ) outHead = 0;
        outCount++;
    }

    void writeBuffer(const uint8_t* buf, int size) 
    {
        while (size > 0)
        {
            this->write(*buf++);
            size--;
        }
    }

public:
    HXFileLog()
    {
        this->outCount = 0;
        this->outHead = 0;
    }

    int flush()
    {
        if ( this->outCount == 0 ) return 0;

        File logFile = SD_MMC.open("/log.txt", FILE_APPEND);
        if ( !logFile ) 
        {
            Serial.println("ERROR: Unable to open log file!");
            return 0;
        }

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

            logFile.write(&outBuffer[p], c); 
            outCount -= c;
            cout += c;
        }

        logFile.close();

        return cout;
    }

    void printf(const char *format, ...)
    {
        char loc_buf[128];
        char * temp = loc_buf;
        va_list arg;
        va_list copy;
        va_start(arg, format);
        va_copy(copy, arg);
        int len = vsnprintf(temp, sizeof(loc_buf), format, copy);
        va_end(copy);
        if(len < 0) 
        {
            va_end(arg);
            return;
        }
        if(len >= (int)sizeof(loc_buf))
        {  
            temp = (char*) malloc(len+1);
            if(temp == NULL) 
            {
                va_end(arg);
                return;
            }
            len = vsnprintf(temp, len+1, format, arg);
        }
        va_end(arg);

        if ( this->getAvailableForWrite() < len )
        {
            this->flush();
        }
        if ( this->getAvailableForWrite() >= len )
        {
            this->writeBuffer((uint8_t*)temp, len);
            Serial.write((uint8_t*)temp, len);
        }

        if(temp != loc_buf)
        {
            free(temp);
        }
    }

    ~HXFileLog()
    {
        flush();
    }
};



