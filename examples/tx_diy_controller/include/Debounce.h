#pragma once

#include <Arduino.h>

#include <stdio.h>
#include <stdarg.h>

//=====================================================================
//=====================================================================
class Debounce
{
private:

    uint32_t firstDebounceMS;
    uint32_t debounceMS;
    uint32_t repeatMS;

    uint32_t lastCheckTime;
    uint32_t lastRepeatTime;
    bool bFirst;
    bool lastState;

protected:    

    virtual bool readState( bool lastState ) = 0;

    virtual void onDebouncedChange( bool state )
    {
    } 

    virtual void onRepeat( bool state )
    {
    } 

public:

    Debounce( bool assumedState, uint32_t firstDebounceMS, uint32_t debounceMS, uint32_t repeatMS )
    {
        this->firstDebounceMS = firstDebounceMS;
        this->debounceMS = debounceMS;
        this->repeatMS = repeatMS;
        
        this->lastState = assumedState;
        bFirst = true;
        this->lastCheckTime = millis();
        this->lastRepeatTime = this->lastCheckTime;
    };

    void loop()
    {
        uint32_t t = millis();

        bool b = this->readState( this->lastState );
        
        if ( b != this->lastState )
        {
            if ( ( this->lastCheckTime + ( bFirst? this->firstDebounceMS : this->debounceMS ) ) < t)        
            {
                this->onDebouncedChange( b );
                
                this->bFirst = false;
                this->lastState = b;
                this->lastCheckTime = t;
            }
            this->lastRepeatTime = t;
        }

        if ( ( this->lastRepeatTime + this->repeatMS ) < t)        
        {
            this->onRepeat( this->lastState );
            this->lastRepeatTime = t;
        }

    }
};

