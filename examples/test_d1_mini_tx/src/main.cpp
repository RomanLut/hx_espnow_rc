#include <Arduino.h>
#include "HX_ESPNOW_RC_Transmitter.h"

#define USE_WIFI_CHANNEL 3

#define TELEMETRY_RATE  4192  // keep rate b/sec

HXRCTransmitter hxrcTransmitter;

unsigned long lastStats = millis();

uint16_t errorCount = 0;

uint8_t incomingTelVal = 0;
uint8_t outgoingTelVal = 0;

uint16 sentCount = 0;
unsigned long rateTime;

//=====================================================================
//=====================================================================
void processIncomingTelemetry()
{
  auto& incomingTelemetry = hxrcTransmitter.getIncomingTelemetryBufffer();

  //we expect that telemetry contains stream of increasing numbers
  while (incomingTelemetry.getCount() != 0)
  {
    uint8_t v = incomingTelemetry.remove();
    if ( incomingTelVal != v )
    {
      incomingTelVal = v;
      errorCount++;
    }
    incomingTelVal++;
  }
}

//=====================================================================
//=====================================================================
void fillOutgoingTelemetry()
{
  auto& outgoingTelemetry = hxrcTransmitter.getOutgoingTelemetryBufffer();

  //fill stream with increasing numbers
  while (outgoingTelemetry.getFreeCount() != 0)
  {
    outgoingTelemetry.insert(outgoingTelVal);
    outgoingTelVal++;
    sentCount++;

    //do not fill buffer randomly
    if ( rand() < RAND_MAX / 10 )
    {
      break;
    }

    //keep max rate
    if ( sentCount > TELEMETRY_RATE / 10 )
    {
      break;
    }
  }

  unsigned long t = millis();
  if ( t - rateTime > 100 )
  {
    rateTime = t;
    sentCount = 0;
  }

}


//=====================================================================
//=====================================================================
void setChannels()
{
  //fill channels with random values 1000..2000
  //last channel should contain summ of all values clamped to range 1000...2000

  uint16_t sum = 0;
  for ( int i = 0; i < HXRC_CHANNELS-1; i++ )
  {
    uint16_t r = 1000 + ((uint32_t)rand()) * 1000 / RAND_MAX;
    hxrcTransmitter.setChannelValue( i, r );
    sum += r;
  }

  sum %= 1000;
  sum += 1000;
  hxrcTransmitter.setChannelValue( HXRC_CHANNELS-1, sum);
}

//=====================================================================
//=====================================================================
void setup()
{
  Serial.begin(115200);
  Serial.println("Start");

  //uint8_t peer_mac[6] = {0x30, 0xAE, 0xA4, 0x99, 0x28, 0xB5}; //quad ap
  uint8_t peer_mac[] = {0x24, 0x62, 0xAB, 0xCA, 0xAA, 0xDC}; //ttgo display sta

  hxrcTransmitter.init(
      HXRCConfig(
          USE_WIFI_CHANNEL,
          peer_mac,
          false,
          -1, false));

  //init SoftAp after HXRC
  //Wifi Channel is global setting for AP, STA and ESP-NOW.
  //If you need AP on this device, initialize it on the same channel that ESP-NOW is using.
  //Station usage is limited. Station will change channel to router AP Channel on connect.
  //It will work only if you ensure that router AP is using the same channels as ESP-NOW.

  WiFi.softAP("hxrct", NULL, USE_WIFI_CHANNEL);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); //led pin is inverted

  rateTime = millis();
}

//=====================================================================
//=====================================================================
void loop()
{
  setChannels();
  processIncomingTelemetry();
  fillOutgoingTelemetry();

  hxrcTransmitter.loop();

  if (millis() - lastStats > 1000)
  {
    lastStats = millis();
    hxrcTransmitter.getTransmitterStats().printStats();
    hxrcTransmitter.getReceiverStats().printStats();
    Serial.print("Errors: ");
    Serial.println(errorCount);
  }

  if (errorCount != 0)
  {
    digitalWrite(LED_BUILTIN, millis() & 128 ? HIGH : LOW);
  }
}