#pragma once

/*
Pinout:
https://randomnerdtutorials.com/esp32-pinout-reference-gpios/
*/

//#define TELEMETRY_BAUDRATE 115200
#define TELEMETRY_BAUDRATE 57600

//PPM output pin
#define PPM_PIN 19

//number of channels in PPM packet, 4...16
#define PPM_CHANNELS_COUNT 16

//uncommenct to inject RSSI into last channel
#define INJECT_RSSI

//=============================================================================
//Receiver binding
#define USE_WIFI_CHANNEL 3
#define USE_KEY 0 

#define USE_LR_MODE true

//if there is not transmitter connection after powerup to the specified time,
//receiver will switch from LR to nomal mode to show AP and allow OTA updates
//set to 0 to disable
#define NORMAL_MODE_DELAY_MS 60*1000
