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
  }
}


//=====================================================================
//=====================================================================
void setup()
{
  hxSBUSEncoder.init( Serial, 1, USBSERIAL_BAUDRATE, USBSERIAL_PARITY, USBSERIAL_INVERTED_SBUS );

  hxrcSlave.init(
      HXRCConfig(
          USE_WIFI_CHANNEL,
          USE_KEY,
          -1, false));

  //REVIEW: receiver does not work if AP is not initialized?
  WiFi.softAP("hxrcrusbsbus", NULL, USE_WIFI_CHANNEL);

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

  hxSBUSEncoder.loop( Serial );
}

//=====================================================================
//=====================================================================
void loop()
{
  hxrcTelemetrySerial.flushIn();
  processIncomingTelemetry();

  hxrcSlave.loop();

  updateSBUSOutput();

  if ( hxrcSlave.getReceiverStats().isFailsafe() )
  {
    ArduinoOTA.handle();  
  }
}