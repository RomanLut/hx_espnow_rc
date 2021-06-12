#include <Arduino.h>
#include "HX_ESPNOW_RC_Transmitter.h"

// REPLACE WITH THE MAC Address of your receiver 
//quad {0x30, 0xAE, 0xA4, 0x99, 0x28, 0xB4} - STA
//quad {0x30, 0xAE, 0xA4, 0x99, 0x28, 0xB5} - AP
//uint8_t broadcastAddress[] = {0x30, 0xAE, 0xA4, 0x99, 0x28, 0xB5};

//doit esp32dev v1 7C:9E:BD:F5:0D:6D sta
//uint8_t broadcastAddress[] = {0x7c, 0x9e, 0xbd, 0xf5, 0x0d, 0x6d};

//esp32cam sta F0:08:D1:CC:B3:11
//uint8_t broadcastAddress[] = {0xf0, 0x08, 0xd1, 0xcc, 0xb3, 0x11};

// REPLACE WITH THE MAC Address of your receiver 
//ttgo-display 24:62:AB:CA:AA:DD - AP
//uint8_t broadcastAddress[] = {0x24, 0x62, 0xAB, 0xCA, 0xAA, 0xDD};


//#define LED 33  //ttgo

HXRCTransmitter hxrcTransmitter;

unsigned long lastStats = millis();

//=====================================================================
//=====================================================================
void setup() 
{
    Serial.begin(115200);
    Serial.println("Start");

    uint8_t peer_mac[6] = {0x30, 0xAE, 0xA4, 0x99, 0x28, 0xB5}; //quad ap

    hxrcTransmitter.init( 
      HXRCConfig( 
        3,
        peer_mac,
        LED_BUILTIN, true 
      )
    );
}

//=====================================================================
//=====================================================================
void loop() 
{
  hxrcTransmitter.loop();

  if (millis() - lastStats > 1000)
  {
    lastStats = millis();
    hxrcTransmitter.getTransmitterStats().printStats();
    hxrcTransmitter.getReceiverStats().printStats();
  }
}