#include <Arduino.h>
//#include <BluetoothSerial.h>

#include "HX_ESPNOW_RC_Master.h"
#include "HX_ESPNOW_RC_SerialBuffer.h"
#include "hx_sbus_decoder.h"
#include "HC06Interface.h"

#define LED_PIN  2
#define USE_SERIAL1_RX_PIN    13
#define USE_SERIAL2_TX_PIN    18


#define USE_WIFI_CHANNEL 3
#define USE_KEY 0

HXRCMaster hxrcMaster;
HXRCSerialBuffer<512> hxrcTelemetrySerial( &hxrcMaster );
HXSBUSDecoder sbusDecoder;

//static BLESerial SerialBT;
//static BluetoothSerial SerialBT;
static HC06Interface externalBTSerial;

unsigned long lastStats = millis();

//=====================================================================
//=====================================================================
void processIncomingTelemetry()
{
  while ( hxrcTelemetrySerial.getAvailable() > 0 )
  {
    uint8_t c = hxrcTelemetrySerial.peek();
    if ( externalBTSerial.write( c ) == 0 ) break;
    hxrcTelemetrySerial.read();
  }
}

//=====================================================================
//=====================================================================
void fillOutgoingTelemetry()
{
  
  while ( (externalBTSerial.available() > 0) && (hxrcTelemetrySerial.getAvailableForWrite() > 0) )
  {
    uint8_t c = externalBTSerial.read();
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
      hxrcMaster.setChannelValue( i, r );
    }
  }

  //use channel 16 to transmit failsafe flag
  hxrcMaster.setChannelValue( 15, sbusDecoder.isFailsafe() ? 1 : 0 );
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

  externalBTSerial.init();

/*
  if ( !SerialBT.setPin("1234") ) 
  {
    Serial.println("An error occurred setting Bluetooth pin");
  }*/

  hxrcMaster.init(
      HXRCConfig(
          USE_WIFI_CHANNEL,
          USE_KEY,
          false,
          -1, false));

  WiFi.softAP("hxrct", NULL, USE_WIFI_CHANNEL);

  /*
  if ( !SerialBT.begin("HXRCBLE") ) 
  {
    Serial.println("An error occurred initializing Bluetooth");
  }
  */

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

  hxrcMaster.loop();

  if (millis() - lastStats > 1000)
  {
    lastStats = millis();
    hxrcMaster.getTransmitterStats().printStats();
    hxrcMaster.getReceiverStats().printStats();
  }


/*
  if (millis() - lastStats > 1000)
  {
    lastStats = millis();
    sbusDecoder.dump();
  }
*/

 }


