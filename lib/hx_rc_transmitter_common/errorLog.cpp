#include "errorLog.h"

#include <SPIFFS.h> 

#include <stdio.h>
#include <stdarg.h>

#include "HX_ESPNOW_RC_common.h"

ErrorLog ErrorLog::instance;

//=====================================================================
//=====================================================================
ErrorLog::ErrorLog() 
{

}

//=====================================================================
//=====================================================================
void ErrorLog::write( const char* msg )
{
    HXRCLOG.print(msg);
    this->hasError = true;

    File logFile = SPIFFS.open("/errorLog.txt", FILE_APPEND);
    if (!logFile) 
    {
        HXRCLOG.println("- failed to open log file for writing");
        return;
    }
    logFile.print(msg);
    logFile.close();

}

//=====================================================================
//=====================================================================
void ErrorLog::writeOnce( const char* msg )
{
    if ( this->writeOnceDisabled ) return;
    this->write( msg );
}

//=====================================================================
//=====================================================================
void ErrorLog::disableWriteOnce()
{
    this->writeOnceDisabled = true;
}

//=====================================================================
//=====================================================================
bool ErrorLog::getHasError()
{
    bool res = this->hasError && !this->hadError;

    if ( res)
    {
    this->hadError = true;
    }
    return res;
}
