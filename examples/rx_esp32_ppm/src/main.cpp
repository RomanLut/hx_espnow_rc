#include <Arduino.h>
#include "HX_ESPNOW_RC_Slave.h"
#include "rx_config.h"
#include "hx_ppm_encoder.h"
#include "HX_ESPNOW_RC_SerialBuffer.h"

#include <esp_task_wdt.h>

#include <ArduinoOTA.h>

#define WDT_TIMEOUT_SECONDS 3  

HXRCSlave hxrcSlave;
HXRCSerialBuffer<512> hxrcTelemetrySerial( &hxrcSlave );
HXPPMEncoder hxPPMEncoder;

unsigned long lastStats = millis();

//0 - got connection once
//1 - switched to normal mode
//2 - waiting for connection
//other - millis() at startup
uint8_t state = 2;
unsigned long startTime = millis();

//=====================================================================
//=====================================================================
void processIncomingTelemetry()
{
  while ( hxrcTelemetrySerial.getAvailable() > 0 && Serial.availableForWrite() > 0)
  {
    uint8_t c = hxrcTelemetrySerial.read();
    Serial.write( c );
  }
}

//=====================================================================
//=====================================================================
void fillOutgoingTelemetry()
{
  while ( (Serial.available() > 0) && (hxrcTelemetrySerial.getAvailableForWrite() > 0) )
  {
    uint8_t c = Serial.read();
    hxrcTelemetrySerial.write(c);
  }
}

//=====================================================================
//=====================================================================
void onOTAprogress( uint a, uint b )  
{
  esp_task_wdt_reset();
}

//=====================================================================
//=====================================================================
void setup()
{
  esp_task_wdt_init(WDT_TIMEOUT_SECONDS, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch

  Serial.begin(TELEMETRY_BAUDRATE);
//  Serial.println("Start");

  hxPPMEncoder.init( PPM_CHANNELS_COUNT, PPM_PIN );

  hxrcSlave.init(
      HXRCConfig(
          USE_WIFI_CHANNEL,
          USE_KEY,
          USE_LR_MODE,
          LED_BUILTIN, false));

  //REVIEW: receiver does not work if AP is not initialized?
  WiFi.softAP("hxrcrsbus", NULL, USE_WIFI_CHANNEL);

  ArduinoOTA.onProgress(&onOTAprogress);
  ArduinoOTA.begin();  
}

//=====================================================================
//=====================================================================
void updatePPMOutput()
{
  //set failsafe flag
  bool failsafe = hxrcSlave.getReceiverStats().isFailsafe();
  hxPPMEncoder.setFailsafe( failsafe);
  
  HXRCChannels channels = hxrcSlave.getChannels();
  for ( int i = 0; i < PPM_CHANNELS_COUNT; i++)
  {
    hxPPMEncoder.setChannelValue( i, channels.getChannelValue(i) );
  }

#ifdef INJECT_RSSI
  //inject RSSI into channel 8
  hxPPMEncoder.setChannelValue( PPM_CHANNELS_COUNT-1, 1000 + ((uint16_t)hxrcSlave.getReceiverStats().getRSSI())*10 );
#endif

  if ( !failsafe ) 
  {
    if ( state == 1 )
    {
      Serial.println("Rebooting to LR mode");
      delay(100);
      ESP.restart();
      delay(1000);
    }
    state = 0;
  }

  hxPPMEncoder.commit();
}

//=====================================================================
//=====================================================================
void loop()
{
  esp_task_wdt_reset();

  hxrcTelemetrySerial.flushIn();
  processIncomingTelemetry();
  
  fillOutgoingTelemetry();
  hxrcTelemetrySerial.flushOut();

  hxrcSlave.setA2(hxrcSlave.getReceiverStats().getRSSI());

  hxrcSlave.loop();

/*
  if (millis() - lastStats > 1000)
  {
    lastStats = millis();

    Serial.print("Peer:");
    HXRCPrintMac(hxrcSlave.getPeerMac());
    
    Serial.print(" packets:");
    Serial.print(capture.packetsCount);

    Serial.print(" rssi:");
    Serial.print(capture.rssi);

    Serial.print(" noise_floor:");
    Serial.print(capture.noiseFloor);

    Serial.print(" rate:");
    Serial.println(capture.rate);

    hxrcSlave.getTransmitterStats().printStats();
    hxrcSlave.getReceiverStats().printStats();
  }
*/
  updatePPMOutput();

  if ( hxrcSlave.getReceiverStats().isFailsafe() )
  {
    ArduinoOTA.handle();  
  }

  if ( 
      USE_LR_MODE &&
      (state == 2) && 
      ((NORMAL_MODE_DELAY_MS) > 0) &&
      ((millis() - startTime) > (NORMAL_MODE_DELAY_MS) ) 
      )
  {
    //if there is no transmitter connection after 1 minute after powerup, and LR more is enabled, 
    //switch to normal mode to show AP and allow OTA updates

    Serial.println("Switching to normal mode");

    state = 1;

    if (esp_wifi_set_protocol (WIFI_IF_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N ) != ESP_OK)
    {
      Serial.println("HXRC: Error: Failed to enable normal mode");
    }
 }
}
 