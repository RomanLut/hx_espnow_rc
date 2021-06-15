#include <Arduino.h>
#include <TFT_eSPI.h>

#include "HX_ESPNOW_RC_Receiver.h"

#define USE_WIFI_CHANNEL 3
#define LED_BUILTIN 33


HXRCReceiver hxrcReceiver;

unsigned long lastStats = millis();

TFT_eSPI tft = TFT_eSPI(135, 240);

char buff[512];

uint16_t y = 0;

uint16_t errorCount = 0;

uint8_t incomingTelVal = 0;
uint8_t outgoingTelVal = 0;

//=====================================================================
//=====================================================================
void processIncomingTelemetry()
{
  auto& incomingTelemetry = hxrcReceiver.getIncomingTelemetryBufffer();

  //we expect that telemetry contains stream of increasing numbers
  while (incomingTelemetry.getCount() != 0)
  {
    uint8_t v = incomingTelemetry.remove();
    if ( incomingTelVal != v )
    {
      incomingTelVal = v;
      //errorCount++;
    }
    incomingTelVal++;
  }
}

//=====================================================================
//=====================================================================
void fillOutgoingTelemetry()
{
  auto& outgoingTelemetry = hxrcReceiver.getOutgoingTelemetryBufffer();

  //fill stream with increasing numbers
  while (outgoingTelemetry.getFreeCount() != 0)
  {
    outgoingTelemetry.insert(outgoingTelVal);
    outgoingTelVal++;

    //do not fill buffer fully 
    if ( rand() < RAND_MAX / 10 )
    {
      break;
    }
  }
}

//=====================================================================
//=====================================================================
void checkChannels()
{
  uint16_t sum = 0;
  for ( int i = 0; i < HXRC_CHANNELS-1; i++ )
  {
    sum += hxrcReceiver.getChannelValue( i );
  }

  sum %= 1000;
  sum += 1000;

  if ( hxrcReceiver.getChannelValue( HXRC_CHANNELS-1) != sum )
  {
      errorCount++;
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
    Serial.println(errorCount);
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
  drawLine("Packets received: %u     ", prStats.packetsSuccess);
  drawLine("Packets missed: %u     ", prStats.packetsError);
  drawLine("Tel. overflow: %u     ", prStats.telemetryOverflowCount);
  drawLine("In tel.: %ub/s     ", prStats.getTelemetryReceivedSpeed());
  drawLine("Errors: %u     ", errorCount);

}