#include <Arduino.h>

#include "HX_ESPNOW_RC_Slave.h"

#define USE_WIFI_CHANNEL 3

HXRCSlave hxrcReceiver;

unsigned long lastStats = millis();
uint16_t errorCountRC = 0;
uint16_t errorCountTelemetry = 0;

bool gotSync = false;
uint8_t incomingTelVal = 0;
uint8_t outgoingTelVal = 0;

//=====================================================================
//=====================================================================
void processIncomingTelemetry()
{
  uint8_t buffer[100];

  //we expect that telemetry contains stream of increasing numbers

  //max 10 cycles at at time
  //otherwise we will sit here forewer processing fast incoming telemetry stream
  for ( int j = 0; j < 10; j++ )
  {
    uint16_t returnedSize = hxrcReceiver.getIncomingTelemetry( 100, buffer );
    if ( returnedSize == 0 ) break;

    uint8_t* ptr = buffer;
    for ( int i = 0; i < returnedSize; i++ )
    {
      if ( incomingTelVal != *ptr )
      {
        if ( gotSync ) errorCountTelemetry++;
        gotSync = true;
        incomingTelVal = *ptr;
      }
      ptr++;
      incomingTelVal++;
    }
  }
}

//=====================================================================
//=====================================================================
void fillOutgoingTelemetry()
{
  uint8_t buffer[HXRC_SLAVE_TELEMETRY_SIZE_MAX];

  //generate at most HXRC_SLAVE_TELEMETRY_SIZE_MAX bytes on each loop
  uint8_t len = rand() % (HXRC_SLAVE_TELEMETRY_SIZE_MAX+1);

  uint8_t v = outgoingTelVal;
  
  //fill stream with increasing numbers
  for ( int i = 0; i < len; i++ ) buffer[i] = v++;
  
  if ( hxrcReceiver.sendOutgoingTelemetry( buffer, len ))
  {
    outgoingTelVal = v;
  }
}

//=====================================================================
//=====================================================================
void checkChannels()
{
  //no sense to check channels before first packet is received
  if ( hxrcReceiver.getReceiverStats().isFailsafe()) return;

  HXRCChannels channels = hxrcReceiver.getChannels();

  uint16_t sum = 0;
  for ( int i = 0; i < HXRC_CHANNELS-1; i++ )
  {
    sum += channels.getChannelValue( i );
  }

  sum %= 1000;
  sum += 1000;

  if ( channels.getChannelValue( HXRC_CHANNELS-1) != sum )
  {
      errorCountRC++;
  }
}

//=====================================================================
//=====================================================================
void setup()
{
  Serial.begin(115200);
  Serial.println("Start");

  //uint8_t peer_mac[6] = {0x7c, 0x9e, 0xbd, 0xf4, 0xC2, 0x28}; //devkit STA 1
  uint8_t peer_mac[6] = {0x98, 0xf4, 0xAB, 0xfb, 0x11, 0x44};  //d1 mini sta
  //uint8_t peer_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};  //broadcast

  hxrcReceiver.init(
      HXRCConfig(
          USE_WIFI_CHANNEL,
          peer_mac,
          false,
          LED_BUILTIN, false));

    WiFi.softAP("hxrcr", NULL, USE_WIFI_CHANNEL );
}

//=====================================================================
//=====================================================================
void loop()
{
  checkChannels();
  processIncomingTelemetry();
  fillOutgoingTelemetry();

  hxrcReceiver.loop();

  if (millis() - lastStats > 1000)
  {
    lastStats = millis();
    hxrcReceiver.getTransmitterStats().printStats();
    hxrcReceiver.getReceiverStats().printStats();
    Serial.print("Errors: ");
    Serial.println(errorCountRC);
 }
 
}