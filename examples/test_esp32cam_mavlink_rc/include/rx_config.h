#pragma once

/*
Pinout:
https://randomnerdtutorials.com/esp32-pinout-reference-gpios/
*/

//mavlink serial port pins (UART2)
#define MAVLINK_RX_PIN    16
#define MAVLINK_TX_PIN    17

//Use Mavlink v1 ( 8 RC Channels ) or Mavlink v2 (15 RC Channels)
#define USE_MAVLINK_V1 false

//telemetry/mavlink port speed
#define TELEMETRY_BAUDRATE 57600

//send packet every ?ms
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
