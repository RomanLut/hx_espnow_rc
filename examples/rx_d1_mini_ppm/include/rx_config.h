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

GPIO2 - PPM output
GPIO15 - Telemetry TX 
GPIO13 - Telemetry RX
*/

//#define TELEMETRY_BAUDRATE 115200
#define TELEMETRY_BAUDRATE 9600

//Number of channels in PPM packet, 4...16
#define PPM_CHANNELS_COUNT 16

//uncommenct to inject RSSI into last channel
#define INJECT_RSSI

//=============================================================================
//Receiver binding
#define USE_WIFI_CHANNEL 3
#define USE_KEY 0 

