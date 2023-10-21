#include <Arduino.h>
#include "HX_ESPNOW_RC_Slave.h"
#include "rx_config.h"
#include "hx_ppm_encoder.h"
#include "HX_ESPNOW_RC_SerialBuffer.h"

#include <ArduinoOTA.h>

HXRCSlave hxrcSlave;
HXRCSerialBuffer<512> hxrcTelemetrySerial( &hxrcSlave );
HXPPMEncoder hxPPMEncoder;

unsigned long lastStats = millis();

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
void setup()
{
  pinMode( 15, OUTPUT); //D8 TX
  pinMode( 13, INPUT); //D7 RX

  Serial.begin(TELEMETRY_BAUDRATE);
  //Serial.begin(115200);
  
  Serial.swap(); //GPIO15 D8 (TX) and GPIO13 D7 (RX)

  hxPPMEncoder.init( PPM_CHANNELS_COUNT, PPM_PIN );  

  hxrcSlave.init(
      HXRCConfig(
          USE_WIFI_CHANNEL,
          USE_KEY,
          -1, false));  //LED_BUILTIN, true

  hxrcSlave.setA1(42);

  //REVIEW: receiver does not work if AP is not initialized?
  WiFi.softAP("hxrcrppm", NULL, USE_WIFI_CHANNEL);

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
  hxPPMEncoder.setChannelValue( PPM_CHANNELS_COUNT-1, 1000 + ((uint16_t)hxrcSlave.getReceiverStats().getRSSI())*10 );
#endif

  hxPPMEncoder.commit();
}

//=====================================================================
//=====================================================================
void loop()
{
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
    hxrcSlave.getTransmitterStats().printStats();
    hxrcSlave.getReceiverStats().printStats();
  }
*/

  updatePPMOutput();

  if ( hxrcSlave.getReceiverStats().isFailsafe() )
  {
    ArduinoOTA.handle();  
  }
}