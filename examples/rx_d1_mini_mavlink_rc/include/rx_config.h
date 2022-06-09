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

GPIO15 - TX 
GPIO13 - RX
*/

//Use Mavlink v1 ( 8 RC Channels ) or Mavlink v2 (15 RC Channels)
#define USE_MAVLINK_V1 false

//Telemetry/mavlink port speed
#define TELEMETRY_BAUDRATE 115200

//send packet every ?ms
//Note: If RC messages are sent too frequently, inav will not be able to send telemetry.
//Practical rate it 40...50ms. At 20ms, inav is not able to send telemetry at all.
//This can be fixed only by fixing inav sources.
//inav/src/main/telemetry/mavlink.c, remove "if (!incomingRequestServed) ..."
#define MAVLINK_RC_PACKET_RATE_MS   40  //Rate in Hz = 1000 / MAVLINK_RC_PACKET_RATE_MS

//note: 
//Mavlink v1: 18 bytes(message size) * 25(rate) * 10(bits) = 4500 baud
//Mavlink v2: 34 bytes(message size) * 25(rate) * 10(bits) = 8500 baud

//=============================================================================
//Receiver binding
#define USE_WIFI_CHANNEL 3
#define USE_KEY 0 
