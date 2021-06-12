#include <Arduino.h>

#include "HX_ESPNOW_RC_Receiver.h"

HXRCReceiver hxrcReceiver;

unsigned long lastStats = millis();

//=====================================================================
//=====================================================================
void setup()
{
  Serial.begin(115200);
  Serial.println("Start");

  uint8_t peer_mac[6] = {0xf0, 0x08, 0xd1, 0xcc, 0xb3, 0x11};  //esp32cam;

  hxrcReceiver.init(
    HXRCConfig(
      3, 
      peer_mac,
      LED_BUILTIN, false
    )
  );
  }

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

}