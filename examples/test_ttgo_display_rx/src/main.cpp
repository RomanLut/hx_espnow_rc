#include <Arduino.h>
#include <TFT_eSPI.h>

#include "HX_ESPNOW_RC_Slave.h"

#define USE_WIFI_CHANNEL 3
#define LED_BUILTIN 33


HXRCSlave hxrcReceiver;

unsigned long lastStats = millis();

TFT_eSPI tft = TFT_eSPI(135, 240);

char buff[512];

uint16_t y = 0;

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

  tft.init();
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextDatum(ML_DATUM);
  tft.setRotation(0);

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

  hxrcReceiver.loop();

  if (millis() - lastStats > 1000)
  {
    lastStats = millis();
    hxrcReceiver.getTransmitterStats().printStats();
    hxrcReceiver.getReceiverStats().printStats();
    Serial.print("Errors: ");
    Serial.println(errorCountRC);
 }

  y = 0;

  HXRCTransmitterStats& ptStats = hxrcReceiver.getTransmitterStats();
  drawLine("Transmitter Failsafe: %d     ", ptStats.isFailsafe() ? 1 : 0);
  drawLine("Transmitter RSSI: %d     ", ptStats.getRSSI());
  drawLine("Packets sent: %u     ", ptStats.packetsSentTotal);
  drawLine("Packets success: %u     ", ptStats.packetsSentSuccess);
  drawLine("Packets Error: %u     ", ptStats.packetsSentError);
  drawLine("Packets missed: %u     ", ptStats.packetsNotSentInTime);
  drawLine("Out tel.: %ub/s     ", ptStats.getTelemetrySendSpeed());

  y += 14;

  HXRCReceiverStats& prStats = hxrcReceiver.getReceiverStats();
  drawLine("Receiver Failsafe: %d     ", prStats.isFailsafe() ? 1 : 0);
  drawLine("Receiver RSSI: %d     ", prStats.getRSSI());
  drawLineI("Remote RSSI: %i     ", prStats.getRemoteReceiverRSSI());

  sprintf(buff, "Packets received: %u(%u)     ", prStats.packetsSuccess, prStats.packetsRetransmit);
  tft.drawString(buff, 0, y);
  y += 14;

  drawLine("Packets missed: %u     ", prStats.packetsError);
  drawLine("Tel. overflow: %u     ", prStats.telemetryOverflowCount);
  drawLine("In tel.: %ub/s     ", prStats.getTelemetryReceivedSpeed());

  sprintf(buff, "Errors: %u, %u     ", errorCountRC, errorCountTelemetry);
  tft.drawString(buff, 0, y);
 
}