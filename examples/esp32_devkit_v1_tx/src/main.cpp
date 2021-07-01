#include <Arduino.h>
#include <BluetoothSerial.h>

#include "HX_ESPNOW_RC_Master.h"
#include "HX_ESPNOW_RC_SerialBuffer.h"
#include "hx_sbus_decoder.h"

#define LED_PIN  2
#define USE_SERIAL1_RX_PIN    13
#define USE_SERIAL2_TX_PIN    18


#define USE_WIFI_CHANNEL 3
//uint8_t peer_mac[] = {0x24, 0x62, 0xAB, 0xCA, 0xAA, 0xDC}; //ttgo display sta
//uint8_t peer_mac[6] = {0x30, 0xAE, 0xA4, 0x99, 0x28, 0xB4};
uint8_t peer_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
const char* key = "HXRC_DEFAULT_KEY"; //16 bytes

HXRCMaster hxrcTransmitter;
HXRCSerialBuffer<512> hxrcTelemetrySerial( &hxrcTransmitter );
HXSBUSDecoder sbusDecoder;

static BluetoothSerial SerialBT;

unsigned long lastStats = millis();

//=====================================================================
//=====================================================================
void processIncomingTelemetry()
{
  while ( hxrcTelemetrySerial.getAvailable() > 0 )
  {
    uint8_t c = hxrcTelemetrySerial.peek();
    if ( SerialBT.write( c ) == 0 ) break;
    hxrcTelemetrySerial.read();
  }
}

//=====================================================================
//=====================================================================
void fillOutgoingTelemetry()
{
  while ( (SerialBT.available() > 0) && (hxrcTelemetrySerial.getAvailableForWrite() > 0) )
  {
    uint8_t c = SerialBT.read();
    hxrcTelemetrySerial.write(c);
  }
}


//=====================================================================
//=====================================================================
void setChannels()
{
  if (!sbusDecoder.isFailsafe())
  {
    //15 channels
    for ( int i = 0; i < HXRC_CHANNELS-1; i++ )
    {
      uint16_t r = sbusDecoder.getChannelValueInRange( i, 1000, 2000 );
      //if ( i == 3 ) Serial.println(r);
      hxrcTransmitter.setChannelValue( i, r );
    }
  }

  //use channel 16 to transmit failsafe flag
  hxrcTransmitter.setChannelValue( 15, sbusDecoder.isFailsafe() ? 1 : 0 );
}

//=====================================================================
//=====================================================================
void initLedPin()
{
  pinMode(LED_PIN,OUTPUT);
  digitalWrite(LED_PIN, LOW );
}

//=====================================================================
//=====================================================================
void setLed( bool value )
{
  digitalWrite(LED_PIN, value ? HIGH : LOW );
}


//=====================================================================
//=====================================================================
void setup()
{
  Serial.begin(115200);
  Serial.println("Start");

  initLedPin();

  sbusDecoder.init(USE_SERIAL1_RX_PIN);

  setLed(true);

  Serial2.begin(57600, SERIAL_8N1, -1, USE_SERIAL2_TX_PIN, true );  //SPORT

  if ( !SerialBT.begin("HXRC") ) 
  {
    Serial.println("An error occurred initializing Bluetooth");
  }

  if ( !SerialBT.setPin("1234") ) 
  {
    Serial.println("An error occurred setting Bluetooth pin");
  }

  hxrcTransmitter.init(
      HXRCConfig(
          USE_WIFI_CHANNEL,
          peer_mac,
          key,
          false,
          -1, false));

  WiFi.softAP("hxrct", NULL, USE_WIFI_CHANNEL);
}

//=====================================================================
//=====================================================================
void loop()
{
  sbusDecoder.loop();
  setChannels();

  hxrcTelemetrySerial.flush();
  processIncomingTelemetry();
  fillOutgoingTelemetry();

  hxrcTransmitter.loop();

  if (millis() - lastStats > 1000)
  {
    lastStats = millis();
    hxrcTransmitter.getTransmitterStats().printStats();
    hxrcTransmitter.getReceiverStats().printStats();
  }


/*
  if (millis() - lastStats > 1000)
  {
    lastStats = millis();
    sbusDecoder.dump();
  }
*/

 }


