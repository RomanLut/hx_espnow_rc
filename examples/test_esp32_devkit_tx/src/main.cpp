#include <Arduino.h>
#include "HX_ESPNOW_RC_Master.h"
#include "HX_ESPNOW_RC_SerialBuffer.h"

#define USE_WIFI_CHANNEL 3
#define USE_KEY 0

#define TELEMETRY_RATE  4192  // keep rate b/sec

#define TEST_SERIALBUFFER

HXRCMaster hxrcMaster;

unsigned long lastStats = millis();

uint16_t errorCountTelemetry = 0;

bool gotSync = false;
uint8_t incomingTelVal = 0;
uint8_t outgoingTelVal = 0;

unsigned long rateTime = millis();
uint16_t rateCounter = 0;

#ifdef TEST_SERIALBUFFER
HXRCSerialBuffer<256> hxrcTelemetrySerial( &hxrcMaster );
#endif

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
#ifdef TEST_SERIALBUFFER
    uint16_t returnedSize = min( 100, (int)hxrcTelemetrySerial.getAvailable() );
    for ( int i = 0; i < returnedSize; i++)
    {
      buffer[i] = hxrcTelemetrySerial.read();
    }
#else
    uint16_t returnedSize = hxrcMaster.getIncomingTelemetry( 100, buffer );
#endif    
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
void processSimpleTelemetry()
{
  if ( hxrcMaster.getReceiverStats().isFailsafe() ) return;

  if ( hxrcMaster.getA1() != ~hxrcMaster.getA2() )
  {
    errorCountTelemetry++;
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

  //generate at most HXRC_MASTER_TELEMETRY_SIZE_MAX bytes on each loop
  uint8_t len = rand() % (HXRC_MASTER_TELEMETRY_SIZE_MAX+1);

  uint8_t v = outgoingTelVal;

  uint8_t buffer[HXRC_MASTER_TELEMETRY_SIZE_MAX];

  //fill stream with increasing numbers
  for ( int i = 0; i < len; i++ ) buffer[i] = v++;

#ifdef TEST_SERIALBUFFER
  if ( hxrcTelemetrySerial.getAvailableForWrite() >= len )
  {
    for ( int i = 0; i < len; i++ ) hxrcTelemetrySerial.write(buffer[i]);
    outgoingTelVal = v;
    rateCounter += len;
  }
#else
  if ( hxrcMaster.sendOutgoingTelemetry( buffer, len ))
  {
    outgoingTelVal = v;
    rateCounter += len;
  }
#endif  
}


//=====================================================================
//=====================================================================
void setChannels()
{
  //fill channels with random values 1000..2000
  //last channel should contain summ of all values clamped to range 1000...2000

  uint16_t sum = 0;
  for ( int i = 0; i < HXRC_CHANNELS_COUNT-1; i++ )
  {
    uint16_t r = 1000 + ((uint32_t)rand()) * 1000 / RAND_MAX;
    hxrcMaster.setChannelValue( i, r );
    sum += r;
  }

  sum %= 1000;
  sum += 1000;
  hxrcMaster.setChannelValue( HXRC_CHANNELS_COUNT-1, sum);
}

//=====================================================================
//=====================================================================
void setup()
{
  Serial.begin(115200);
  Serial.println("Start");

  hxrcMaster.init(
      HXRCConfig(
          USE_WIFI_CHANNEL,
          USE_KEY,
          false, //LR mode
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
  processSimpleTelemetry();
  fillOutgoingTelemetry();

/*
 #ifdef TEST_SERIALBUFFER
  hxrcTelemetrySerial.flush();
 #endif 
*/

  hxrcMaster.loop();

  if (millis() - lastStats > 1000)
  {
    lastStats = millis();
    
    Serial.print("=== Peer:");
    HXRCPrintMac(hxrcMaster.getPeerMac());
    Serial.print("\n");
    
    hxrcMaster.getTransmitterStats().printStats();
    hxrcMaster.getReceiverStats().printStats();
    
    Serial.print("Telemetry stream errors: ");
    Serial.println(errorCountTelemetry);
  }

  if (errorCountTelemetry > 0) 
  {
    digitalWrite(LED_BUILTIN, millis() & 128 ? HIGH : LOW);
  }
}