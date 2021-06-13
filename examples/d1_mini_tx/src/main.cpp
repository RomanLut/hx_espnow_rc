#include <Arduino.h>
#include "HX_ESPNOW_RC_Transmitter.h"

HXRCTransmitter hxrcTransmitter;

unsigned long lastStats = millis();

#define USE_WIFI_CHANNEL 3

//=====================================================================
//=====================================================================
void setup() 
{
    Serial.begin(115200);
    Serial.println("Start");

    //uint8_t peer_mac[6] = {0x30, 0xAE, 0xA4, 0x99, 0x28, 0xB5}; //quad ap
    uint8_t peer_mac[] = {0x24, 0x62, 0xAB, 0xCA, 0xAA, 0xDC};  //ttgo display sta

    hxrcTransmitter.addIncomingTelemetryCallback( [](void* parm, HXRCTransmitter& transmitter)
    {
      auto incomingTelemetry = transmitter.getIncomingTelemetryBufffer();

      while ( incomingTelemetry.getCount() != 0 )      
      {
        incomingTelemetry.remove();
      }
    },NULL);

    hxrcTransmitter.init( 
      HXRCConfig( 
        USE_WIFI_CHANNEL,
        peer_mac,
        false,
        LED_BUILTIN, true 
      )
    );

    //init SoftAp after HXRC
    //Wifi Channel is global setting for AP, STA and ESP-NOW.
    //If you need AP on this device, initialize it on the same channel that ESP-NOW is using.
    //Station usage is limited. Station will change channel to router AP Channel on connect.
    //It will work only if you ensure that router AP is using the same channels as ESP-NOW.

    WiFi.softAP("hxrct", NULL, USE_WIFI_CHANNEL );
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