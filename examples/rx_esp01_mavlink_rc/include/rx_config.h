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

//Use Mavlink v1 ( 8 RC Channels ) or Mavlink v2 (15 RC Channels)
#define USE_MAVLINK_V1 false

//Telemetry/mavlink port speed
#define TELEMETRY_BAUDRATE 115200

//send packet every ?ms
//Note for inav: "Serial receiver half-duplex" should be set to "OFF", otherwise inav will not be able to send telemetry.
#define MAVLINK_RC_PACKET_RATE_MS   40  //Rate in Hz = 1000 / MAVLINK_RC_PACKET_RATE_MS

//Declare chanel which is used to switch function of shared port in inav.
//See telemetry_channel CLI setting in inav.
//Channel value < 1250 disables MAVLINK_RC messages and switches telemetry stream to transparent bidirectional mode.
//Note that it is not be possible to switch UART back to telemetry mode because MAVLINK_RC messages will not be sent to FC anymore; channel value on FC is stuck in MSP mode.
//The only way to switch back is to reset FC.
//Thus it is not possible to use ARM switch for this purpoce. 
//Number is one based index!!! (channel 1 = 1)
//set to 0 to disable
#define MSP_SWITCH_CHANNEL 0

//note: 
//Mavlink v1: 18 bytes(message size) * 25(rate) * 10(bits) = 4500 baud
//Mavlink v2: 34 bytes(message size) * 25(rate) * 10(bits) = 8500 baud

//=============================================================================
//Receiver binding
#define USE_WIFI_CHANNEL 3
#define USE_KEY 0 
