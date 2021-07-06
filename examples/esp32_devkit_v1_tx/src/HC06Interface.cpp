#include "HC06Interface.h"

#define HC06_BAUD_RATE 115200

#define RX_PIN  35
#define TX_PIN  32

//=====================================================================
//=====================================================================
HC06Interface::HC06Interface(int UART) : serial(UART)
{

}

//=====================================================================
//=====================================================================
void HC06Interface::flushResponse()
{
    while ( this->serial.available())
    {
        char c = this->serial.read();
        Serial.print(c);
    }
}

//=====================================================================
//=====================================================================
void HC06Interface::init()
{
    
    /*
    //Uncomment to initialize once if requred,one command at at time
    //Commands for HC-06 SPP 3 module: no need for crlf, type quickly, pause 1 second
    //Comands may vary with version
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

    this->serial.begin(HC06_BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN );
}


//=====================================================================
//=====================================================================
int HC06Interface::available()
{
    return this->serial.available();
}


//=====================================================================
//=====================================================================
int HC06Interface::read()
{
    return this->serial.read();
}

//=====================================================================
//=====================================================================
int HC06Interface::availableForWrite()
{
    return this->serial.availableForWrite();
}
//=====================================================================
//=====================================================================
size_t HC06Interface::write(uint8_t c)
{
    return this->serial.write(c);
}

