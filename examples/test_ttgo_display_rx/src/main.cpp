#include <Arduino.h>
#include <TFT_eSPI.h>

#include "HX_ESPNOW_RC_Slave.h"
#include "HX_ESPNOW_RC_SerialBuffer.h"

#define USE_WIFI_CHANNEL 3

#define USE_KEY   0

#define LED_BUILTIN 33

#define MAX_OUTGOING_TELEMETRY_RATE
#define TEST_SERIALBUFFER

HXRCSlave hxrcSlave;

unsigned long lastStats1 = millis();
unsigned long lastStats2 = millis() + 300;
unsigned long lastStats3 = millis() + 600;

TFT_eSPI tft = TFT_eSPI(135, 240);

char buff[512];

uint16_t y = 0;

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
void setSimpleTelemetry()
{
  uint32_t r = rand() & 0xffff;
  hxrcSlave.setA1((~r & 0xffff ) | (r << 16));
  hxrcSlave.setA2(r | (~r << 16));
}

//=====================================================================
//=====================================================================
void setup()
{
  Serial.begin(115200);
  Serial.println("Start");

  tft.init();
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextDatum(ML_DATUM);
  tft.setRotation(0);

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
void drawLine(const char *text, uint16_t value)
{
  sprintf(buff, text, value);
  tft.drawString(buff, 0, y);
  y += 16;
}

//=====================================================================
//=====================================================================
void drawLineI(const char *text, int16_t value)
{
  sprintf(buff, text, value);
  tft.drawString(buff, 0, y);
  y += 14;
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

  unsigned long t = millis();

  if (t - lastStats1 > 1000)
  {
    lastStats1 = t;

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


  if (t - lastStats2 > 1000)
  {
    lastStats2 = t;

    HXRCTransmitterStats& ptStats = hxrcSlave.getTransmitterStats();
    
    y = 0;
    drawLine("Transmitter Failsafe: %d ", ptStats.isFailsafe() ? 1 : 0);
    drawLine("RSSI: %d     ", ptStats.getRSSI());
    drawLine("Sent: %u     ", ptStats.packetsSentTotal);
    drawLine("Ack: %u     ", ptStats.packetsAcknowledged);
    drawLine("Error: %u     ", ptStats.packetsSentError);
    drawLine("Missed: %u     ", ptStats.packetsNotSentInTime);
    drawLine("Out tel.: %ub/s     ", ptStats.getTelemetrySendSpeed());
  }

  if (t - lastStats3 > 1000)
  {
    lastStats3 = t;

    HXRCReceiverStats& prStats = hxrcSlave.getReceiverStats();

    y = 9*14;
    drawLine("Receiver Failsafe: %d ", prStats.isFailsafe() ? 1 : 0);
    drawLine("RSSI: %d     ", prStats.getRSSI());

    sprintf(buff, "Recv(rty):%u(%u)    ", prStats.packetsReceived, prStats.packetsRetransmit);
    tft.drawString(buff, 0, y);
    y += 14;

    drawLine("Lost: %u     ", prStats.packetsLost);

    sprintf(buff, "Invalid/crc: %u(%u)     ", prStats.packetsInvalid, prStats.packetsCRCError);
    tft.drawString(buff, 0, y);
    y += 14;

    sprintf(buff, "Tel.ovrfl/rs: %u/%u     ", prStats.telemetryOverflowCount, prStats.resyncCount);
    tft.drawString(buff, 0, y);
    y += 14;

    drawLine("In tel.: %ub/s     ", prStats.getTelemetryReceivedSpeed());

    sprintf(buff, "Errors: %u, %u     ", errorCountRC, errorCountTelemetry);
    tft.drawString(buff, 0, y);
  }
 
}