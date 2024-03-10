#include <Arduino.h>
#include "HX_ESPNOW_RC_Slave.h"
#include "rx_config.h"
#include "hx_sbus_encoder.h"
#include "HX_ESPNOW_RC_SerialBuffer.h"

#include <ArduinoOTA.h>

HXRCSlave hxrcSlave;
HXRCSerialBuffer<512> hxrcTelemetrySerial( &hxrcSlave );
HXSBUSEncoder hxSBUSEncoder;

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
  pinMode( 15, OUTPUT);
  pinMode( 13, INPUT);

  Serial.begin(TELEMETRY_BAUDRATE);

  Serial.swap(); //GPIO15 D8 (TX) and GPIO13 D7 (RX)
  
  hxSBUSEncoder.init( Serial1, 2, 100000, SERIAL_8E2, SBUS_INVERTED );

  hxrcSlave.init(
      HXRCConfig(
          USE_WIFI_CHANNEL,
          USE_KEY,
          -1, false));

  //REVIEW: receiver does not work if AP is not initialized?
  WiFi.softAP("hxrcrsbus", NULL, USE_WIFI_CHANNEL);

  ArduinoOTA.begin();  
}

//=====================================================================
//=====================================================================
void updateSBUSOutput()
{
  //set failsafe flag
  bool failsafe = hxrcSlave.getReceiverStats().isFailsafe();
  hxSBUSEncoder.setFailsafe( failsafe);
  
  //inject RSSI into channel 16
  hxSBUSEncoder.setChannelValue( HXRC_CHANNELS_COUNT-1, 1000 + ((uint16_t)hxrcSlave.getReceiverStats().getRSSI())*10 );

  if ( !failsafe ) //keep last channel values on failsafe
  {
    HXRCChannels channels = hxrcSlave.getChannels();
    for ( int i = 0; i < HXRC_CHANNELS_COUNT-1; i++)
    {
      hxSBUSEncoder.setChannelValue( i, channels.getChannelValue(i) );
    }
  }

  hxSBUSEncoder.loop( Serial1 );
}

//=====================================================================
//=====================================================================
void loop()
{
  hxrcTelemetrySerial.flushIn();
  processIncomingTelemetry();
  
  fillOutgoingTelemetry();
  hxrcTelemetrySerial.flushOut();

  //Example: send custom S.PORT telemetry: A1 and A2 values
  hxrcSlave.setA1(42);
  hxrcSlave.setA2(hxrcSlave.getReceiverStats().getRSSI());

  hxrcSlave.loop();


/*
  //Note: Stats are written to serial. Serial is reconfigured to D8,D7
  if (millis() - lastStats > 1000)
  {
    lastStats = millis();
    hxrcSlave.getTransmitterStats().printStats();
    hxrcSlave.getReceiverStats().printStats();
  }
*/

  updateSBUSOutput();

  if ( hxrcSlave.getReceiverStats().isFailsafe() )
  {
    ArduinoOTA.handle();  
  }
}