#include "HC06Interface.h"

#define HC06_BAUD_RATE 115200

//=====================================================================
//=====================================================================
HC06Interface::HC06Interface() 
{
}

//=====================================================================
//=====================================================================
void HC06Interface::flushResponse()
{
    while ( this->serial->available())
    {
        char c = this->serial->read();
        Serial.print(c);
    }
}

//=====================================================================
//=====================================================================
void HC06Interface::init(HardwareSerial* serial,int port_rx_pin, int port_tx_pin)
{
    this->serial = serial; 
    
#if false    
	//HC-06 initialization
    //Uncomment to configure HC-06 moduke once if requred
	//
    //Commands for HC-06 SPP 3 module: no need for crlf, type quickly, pause 1 second
    //Commands may vary with HC-06 module version
    //Default baud rate is 9600
    Serial.println("Initializing HC06");
    
    this->serial->begin(9600, SERIAL_8N1, port_rx_pin, port_tx_pin );
    delay(100);
    this->serial->print( "AT+VERSION" );
    delay(1100);
    this->flushResponse();
    this->serial->print( "AT+NAMEHXRC" );
    delay(1100);
    this->flushResponse();
    this->serial->print("AT+PIN1234");
    delay(1100);
    this->flushResponse();
    this->serial->print("AT+BAUD8");  //4-9600, 7-57600, 8 -115200

    delay(1100);
    this->flushResponse();
#endif    
   
    this->serial->begin(HC06_BAUD_RATE, SERIAL_8N1, port_rx_pin, port_tx_pin);
}


//=====================================================================
//=====================================================================
int HC06Interface::available()
{
    return this->serial->available();
}


//=====================================================================
//=====================================================================
int HC06Interface::read()
{
    return this->serial->read();
}

//=====================================================================
//=====================================================================
int HC06Interface::availableForWrite()
{
    return this->serial->availableForWrite();
}
//=====================================================================
//=====================================================================
size_t HC06Interface::write(uint8_t c)
{
    return this->serial->write(c);
}

