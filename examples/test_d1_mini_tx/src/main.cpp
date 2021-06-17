#include <Arduino.h>
#include "HX_ESPNOW_RC_Master.h"

#define USE_WIFI_CHANNEL 3

#define TELEMETRY_RATE  4192  // keep rate b/sec

HXRCMaster hxrcTransmitter;

unsigned long lastStats = millis();

uint16_t errorCountTelemetry = 0;

bool gotSync = false;
uint8_t incomingTelVal = 0;
uint8_t outgoingTelVal = 0;

unsigned long rateTime = millis();
uint16_t rateCounter = 0;

//=====================================================================
//=====================================================================
void processIncomingTelemetry()
{
  uint8_t buffer[100];

  //we expect that telemetry contains stream of increasing numbers

  //max 10 cycles at at time
  //otherwise we will sit here forewer processinf fast incoming telemetry stream
  for ( int j = 0; j < 10; j++ )
  {
    uint16_t returnedSize = hxrcTransmitter.getIncomingTelemetry( 100, buffer );
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
  unsigned long t = millis();
  if ( t - rateTime > 100 )
  {
    rateTime = t;
    rateCounter = 0;
  }

  if ( rateCounter > 100)  //1kb/sec
  {
    return;
  }

  uint8_t buffer[HXRC_MASTER_TELEMETRY_SIZE_MAX];

  //generate at most HXRC_MASTER_TELEMETRY_SIZE_MAX bytes on each loop
  uint8_t len = rand() % (HXRC_MASTER_TELEMETRY_SIZE_MAX+1);

  uint8_t v = outgoingTelVal;
  
  //fill stream with increasing numbers
  for ( int i = 0; i < len; i++ ) buffer[i] = v++;
  
  if ( hxrcTransmitter.sendOutgoingTelemetry( buffer, len ))
  {
    outgoingTelVal = v;
    rateCounter += len;
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
    Serial.println(errorCountTelemetry);
  }

  if (errorCountTelemetry > 0) 
  {
    digitalWrite(LED_BUILTIN, millis() & 128 ? HIGH : LOW);
  }
}