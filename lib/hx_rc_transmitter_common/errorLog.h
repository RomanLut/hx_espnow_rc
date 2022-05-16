#pragma once

#include <Arduino.h>


//=====================================================================
//=====================================================================
class ErrorLog
{
private:
    bool writeOnceDisabled = false;
    bool hasError = false;

public:
    static ErrorLog instance;

    ErrorLog();

    void write( const char* msg);
    void writeOnce( const char* msg);
    void disableWriteOnce();
    
    void signalError();
    bool getHasErrorAndClear();
};


