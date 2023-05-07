#include <Arduino.h>
#include "HX_ESPNOW_RC_Slave.h"
#include "rx_config.h"
#include "hx_mavlink_rc_encoder.h"
#include "HX_ESPNOW_RC_SerialBuffer.h"

#include <ArduinoOTA.h>

HXRCSlave hxrcSlave;
HXRCSerialBuffer<512> hxrcTelemetrySerial( &hxrcSlave );
HXMavlinkRCEncoder hxMavlinkRCEncoder;

unsigned long lastStats = millis();

bool bMSPMode = false;

//=====================================================================
//=====================================================================
bool isInMSPMode()
{
  HXRCChannels channels = hxrcSlave.getChannels();
  return (MSP_SWITCH_CHANNEL > 0) && (channels.getChannelValue(MSP_SWITCH_CHANNEL - 1) < 1250);
}
//=====================================================================
//=====================================================================
void processIncomingTelemetry()
{

  while ( hxrcTelemetrySerial.getAvailable() > 0 && Serial.availableForWrite() > 0)
  {
    //todo: interlieve incoming messages with RC_OVERRIDE messages
    uint8_t c = hxrcTelemetrySerial.read();

    if ( bMSPMode )
    {
      Serial.write(c);
    }
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
  Serial.begin(TELEMETRY_BAUDRATE);
  
  hxMavlinkRCEncoder.init( MAVLINK_RC_PACKET_RATE_MS, USE_MAVLINK_V1 );

  hxrcSlave.init(
      HXRCConfig(
          USE_WIFI_CHANNEL,
          USE_KEY,
          false,
          -1, false));

  hxrcSlave.setA1(42);

  //REVIEW: receiver does not work if AP is not initialized?
  WiFi.softAP("hxrcmavlink", NULL, USE_WIFI_CHANNEL);

  ArduinoOTA.begin();  
}

//=====================================================================
//=====================================================================
void updateOutput()
{
  //set failsafe flag
  bool failsafe = hxrcSlave.getReceiverStats().isFailsafe();
  hxMavlinkRCEncoder.setFailsafe( failsafe);
  
  //inject RSSI into channel 16
  hxMavlinkRCEncoder.setChannelValue( USE_MAVLINK_V1 ? MAVLINK_RC_CHANNELS_COUNT_V1 - 1 : MAVLINK_RC_CHANNELS_COUNT - 1, 1000 + ((uint16_t)hxrcSlave.getReceiverStats().getRSSI())*10 );

  if ( !failsafe ) //keep last channel values on failsafe
  {
    HXRCChannels channels = hxrcSlave.getChannels();
    for ( int i = 0; i < MAVLINK_RC_CHANNELS_COUNT-1; i++)
    {
      hxMavlinkRCEncoder.setChannelValue( i, channels.getChannelValue(i) );
    }
  }

  if (bMSPMode == false )
  {
    if ( hxMavlinkRCEncoder.loop( Serial ) )
    {
      //update mode after channel values are sent to FC
      if (!failsafe)
      {
        bMSPMode = isInMSPMode();
      }
    }
  }
  else
  {
    if (!failsafe)
    {
      bMSPMode = isInMSPMode();
    }
  }
  
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

  updateOutput();

  if ( hxrcSlave.getReceiverStats().isFailsafe() )
  {
    ArduinoOTA.handle();  
  }
}