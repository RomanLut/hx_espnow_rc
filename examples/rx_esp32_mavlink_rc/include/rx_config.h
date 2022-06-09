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

#define USE_LR_MODE true

//if there is not transmitter connection after powerup to the specified time,
//receiver will switch from LR to nomal mode to show AP and allow OTA updates
//set to 0 to disable
#define NORMAL_MODE_DELAY_MS 60*1000
