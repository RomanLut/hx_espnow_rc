#pragma once

/* Wemos D1 Mini
            _________
           |  -.-._. |  
     /RST =o         o= TX GPIO1
     ADC0 =o         o= RX GPIO3
GPIO16 D0 =o         o= D1 GPIO5
GPIO14 D5 =o         o= D2 GPIO4
GPIO12 D6 =o         o= D3 GPIO0
GPIO13 D7 =o         o= D4 GPIO2
GPIO15 D8 =o         o= GND
     3.3V =o         o= 5V
           |___USB___| 

https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/ 

GPIO2 - SBUS output
GPIO15 - TX 
GPIO13 - RX
*/

//output inverted SBUS? 
//inverted SBUS = true ===> normal UART
#define SBUS_INVERTED  false

//#define TELEMETRY_BAUDRATE 115200
#define TELEMETRY_BAUDRATE 9600

//=============================================================================
//Receiver binding
#define USE_WIFI_CHANNEL 3
#define USE_KEY 0 
