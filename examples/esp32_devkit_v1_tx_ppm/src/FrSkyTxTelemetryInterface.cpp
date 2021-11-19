#include "FrSkyTxTelemetryInterface.h"



//=====================================================================
//=====================================================================
FrSkyTxTelemetryInterface::FrSkyTxTelemetryInterface(HardwareSerial* serial) 
{
    this->serial = serial;
}

//=====================================================================
//=====================================================================
void FrSkyTxTelemetryInterface::flushResponse()
{
    while ( this->serial->available())
    {
        char c = this->serial->read();
        Serial.print(c);
    }
}

//=====================================================================
//=====================================================================
void FrSkyTxTelemetryInterface::init()
{
    
    /*
	//HC-06 initialization
    //Uncomment to initialize once if requred,one command at at time
	//
    //Commands for HC-06 SPP 3 module: no need for crlf, type quickly, pause 1 second
    //Comands may vary with HC-06 module version
    //Default baud rate is 9600
    
    this->serial.begin(57600, SERIAL_8N1, RX_PIN, TX_PIN );
    delay(100);
    //this->serial.print( "AT+VERSION" );
    //this->serial.print( "AT+NAMEHXRC" );
    //this->serial.print("AT+PIN1234");
    this->serial.print("AT+BAUD8");  //4-9600, 7-57600, 8 -115200

    delay(1100);
    this->flushResponse();
    */

    this->serial->begin(FRSKY_TX_BAUD_RATE, SERIAL_8N1, FRSKY_TX_RX_PIN, FRSKY_TX_TX_PIN );
}


//=====================================================================
//=====================================================================
int FrSkyTxTelemetryInterface::available()
{
    return this->serial->available();
}


//=====================================================================
//=====================================================================
int FrSkyTxTelemetryInterface::read()
{
    return this->serial->read();
}

//=====================================================================
//=====================================================================
int FrSkyTxTelemetryInterface::availableForWrite()
{
    return this->serial->availableForWrite();
}
//=====================================================================
//=====================================================================
size_t FrSkyTxTelemetryInterface::write(uint8_t c)
{
    return this->serial->write(c);
}

