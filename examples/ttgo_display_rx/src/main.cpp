#include <Arduino.h>
#include <TFT_eSPI.h>

#include "HX_ESPNOW_RC_Receiver.h"

HXRCReceiver hxrcReceiver;

unsigned long lastStats = millis();

TFT_eSPI tft = TFT_eSPI(135, 240);

char buff[512];

uint16_t y = 0;

#define LED_BUILTIN 33

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
          3,
          peer_mac,
          false,
          LED_BUILTIN, false));

    WiFi.softAP("hxrcr", NULL, 3 );
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
  // put your main code here, to run repeatedly:
  hxrcReceiver.loop();

  if (millis() - lastStats > 1000)
  {
    lastStats = millis();
    hxrcReceiver.getTransmitterStats().printStats();
    hxrcReceiver.getReceiverStats().printStats();
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
  drawLineI("Packets received: %u     ", prStats.packetsSuccess);
  drawLineI("Packets missed: %u     ", prStats.packetsError);
  drawLineI("Tel. overflow: %u     ", prStats.telemetryOverflowCount);
  drawLine("In tel.: %ub/s     ", prStats.getTelemetryReceivedSpeed());

}