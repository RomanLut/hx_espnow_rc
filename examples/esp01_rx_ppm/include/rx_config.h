#pragma once

/* ESP-01
            ____________
           |  -.-._.   |  
           |           |
           | **  ****  |
           | **  ****  |
           |     ****  |
           |  o o o o  |         GND   GPIO2   GPIO1   RX      
           |__o o o o__|         TX    CH_EN   RESET   VCC 

https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/ 

*/

//output inverted SBUS? 
//inverted SBUS = true ===> normal UART
#define SBUS_INVERTED  false

//#define TELEMETRY_BAUDRATE 115200
#define TELEMETRY_BAUDRATE 115200

//=============================================================================
//Receiver binding
#define USE_WIFI_CHANNEL 3
#define USE_KEY 0 
