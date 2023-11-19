#pragma once

/*
            [][][][][][][][]
            [5V]      [3.3V]
            [GND]       [16]
 Mavlink RX [12]         [0]         <---/ ---> GND  FLASH Button
 Mavlink TX [13]       [GND]
            [15]     [3.3/5]
            [14]         [3] RX      <---/ ---> GND  REC Button
            [2]          [1] TX
       LAMP [4]      [GND/R]  <-- GND on some boards, RESET/EN (pullup 3.3V) on toher boards
            [  ESP32 CAM   ]
            [][][][][][][][]

Pinout:
https://randomnerdtutorials.com/esp32-cam-ai-thinker-pinout/

4 - LAMP
/33 - red LED
2,14,15 - SD 1 bit mode 4,12,13 - SD 4 bit mode
3 - RXD0 (debug)  
1 - TXD0 (debug)
12 - RXD2 (mavlink)
13 - TDX2 (mavlink)
*/

//true to use Mavlink1 ( TODO: support Mavlink1)
#define USE_MAVLINK_V1 false

//telemetry/mavlink port speed
#define TELEMETRY_BAUDRATE 115200

//send RC packet every ?ms
//Note for inav: "Serial receiver half-duplex" should be set to "OFF", otherwise inav will not be able to send telemetry.
#define MAVLINK_RC_PACKET_RATE_MS   40  //Rate in Hz = 1000 / MAVLINK_RC_PACKET_RATE_MS

//note: 
//Mavlink v1: 18 bytes(message size) * 25(rate) * 10(bits) = 4500 baud
//Mavlink v2: 34 bytes(message size) * 25(rate) * 10(bits) = 8500 baud

//=============================================================================
//Receiver binding
#define USE_WIFI_CHANNEL 3
#define USE_KEY 0 

#define USE_LR_MODE false

//if there is no transmitter connection after powerup to the specified time,
//receiver will switch from LR to normal mode to show AP and allow OTA updates
//set to 0 to disable
#define NORMAL_MODE_DELAY_MS 60*1000