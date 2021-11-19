#include "MavEsp8266Interface.h"

//=====================================================================
//=====================================================================
MavEsp8266Interface::MavEsp8266Interface(HardwareSerial* serial) 
{
    this->serial = serial;
}

//=====================================================================
//=====================================================================
void MavEsp8266Interface::flushResponse()
{
    while ( this->serial->available())
    {
        char c = this->serial->read();
        Serial.print(c);
    }
}

//=====================================================================
//=====================================================================
void MavEsp8266Interface::init()
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

    this->serial->begin(MAVESP8266_BAUD_RATE, SERIAL_8N1, MAVESP8266_RX_PIN, MAVESP8266_TX_PIN );
}


//=====================================================================
//=====================================================================
int MavEsp8266Interface::available()
{
    return this->serial->available();
}


//=====================================================================
//=====================================================================
int MavEsp8266Interface::read()
{
    return this->serial->read();
}

//=====================================================================
//=====================================================================
int MavEsp8266Interface::availableForWrite()
{
    return this->serial->availableForWrite();
}
//=====================================================================
//=====================================================================
size_t MavEsp8266Interface::write(uint8_t c)
{
    return this->serial->write(c);
}

