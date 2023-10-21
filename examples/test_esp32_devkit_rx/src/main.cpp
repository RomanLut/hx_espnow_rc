#include <Arduino.h>

#include "HX_ESPNOW_RC_Slave.h"
#include "HX_ESPNOW_RC_SerialBuffer.h"

#define USE_WIFI_CHANNEL 3
#define USE_KEY 0

#define MAX_OUTGOING_TELEMETRY_RATE
#define TEST_SERIALBUFFER

HXRCSlave hxrcSlave;

unsigned long lastStats = millis();
uint16_t errorCountRC = 0;
uint16_t errorCountTelemetry = 0;

bool gotSync = false;
uint8_t incomingTelVal = 0;
uint8_t outgoingTelVal = 0;

#ifdef TEST_SERIALBUFFER
HXRCSerialBuffer<512> hxrcTelemetrySerial( &hxrcSlave );
#endif

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
#ifdef TEST_SERIALBUFFER
    uint16_t returnedSize = min( 100, (int)hxrcTelemetrySerial.getAvailable() );
    for ( int i = 0; i < returnedSize; i++)
    {
      buffer[i] = hxrcTelemetrySerial.read();
    }
#else
    uint16_t returnedSize = hxrcSlave.getIncomingTelemetry( 100, buffer );
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
void fillOutgoingTelemetry()
{
  uint8_t buffer[HXRC_SLAVE_TELEMETRY_SIZE_MAX];

#ifdef MAX_OUTGOING_TELEMETRY_RATE  
  uint8_t len = HXRC_SLAVE_TELEMETRY_SIZE_MAX;
#else
  uint8_t len = rand() % (HXRC_SLAVE_TELEMETRY_SIZE_MAX+1);
#endif  

  uint8_t v = outgoingTelVal;
  
  //fill stream with increasing numbers
  for ( int i = 0; i < len; i++ ) buffer[i] = v++;
  
#ifdef TEST_SERIALBUFFER
  if ( hxrcTelemetrySerial.getAvailableForWrite() >= len )
  {
    for ( int i = 0; i < len; i++ ) hxrcTelemetrySerial.write(buffer[i]);
    outgoingTelVal = v;
  }
#else
  if ( hxrcSlave.sendOutgoingTelemetry( buffer, len ))
  {
    outgoingTelVal = v;
  }
#endif  
}

//=====================================================================
//=====================================================================
void setSimpleTelemetry()
{
  uint32_t r = rand() & 0xffff;
  hxrcSlave.setA1((~r & 0xffff ) | (r << 16));
  hxrcSlave.setA2(r | (~r << 16));
}

//=====================================================================
//=====================================================================
void checkChannels()
{
  //no sense to check channels before first packet is received
  if ( hxrcSlave.getReceiverStats().isFailsafe()) return;

  HXRCChannels channels = hxrcSlave.getChannels();

  uint16_t sum = 0;
  for ( int i = 0; i < HXRC_CHANNELS_COUNT-1; i++ )
  {
    sum += channels.getChannelValue( i );
  }

  sum %= 1000;
  sum += 1000;

  if ( channels.getChannelValue( HXRC_CHANNELS_COUNT-1) != sum )
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

  HXRCConfig config(
          USE_WIFI_CHANNEL,
          USE_KEY,
          false,     //LR mode
          LED_BUILTIN, false);

//config.wifiPhyRate = WIFI_PHY_RATE_5M_L;
//config.wifiPhyRate = WIFI_PHY_RATE_LORA_500K;
//config.slaveTelemertyPayloadSize = HXRC_SLAVE_TELEMETRY_SIZE_MAX;

  hxrcSlave.init(config);

  WiFi.softAP("hxrcr", NULL, USE_WIFI_CHANNEL );
}

//=====================================================================
//=====================================================================
void loop()
{
  checkChannels();
  processIncomingTelemetry();
  fillOutgoingTelemetry();
  setSimpleTelemetry();

 #ifdef TEST_SERIALBUFFER
  hxrcTelemetrySerial.flush();
 #endif 

  hxrcSlave.loop();

  if (millis() - lastStats > 1000)
  {
    lastStats = millis();

    Serial.print("=== Peer:");
    HXRCPrintMac(hxrcSlave.getPeerMac());
    Serial.print("\n");
    
    hxrcSlave.getTransmitterStats().printStats();
    hxrcSlave.getReceiverStats().printStats();
    Serial.print("Errors: ");
    Serial.print(errorCountRC);
    Serial.print(",");
    Serial.println(errorCountTelemetry);
 }
 
}