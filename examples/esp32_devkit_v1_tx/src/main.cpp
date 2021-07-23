#include <Arduino.h>
//#include <BluetoothSerial.h>
#include <SPI.h>

#include <stdio.h>
#include <stdarg.h>

#include "HX_ESPNOW_RC_Master.h"
#include "HX_ESPNOW_RC_SerialBuffer.h"
#include "hx_sbus_decoder.h"

#include "tx_config.h"
#include "HC06Interface.h"

#include "smartport.h"

HXRCMaster hxrcMaster;
HXRCSerialBuffer<512> hxrcTelemetrySerial( &hxrcMaster );
HXSBUSDecoder sbusDecoder;
static HC06Interface externalBTSerial(&Serial2);

#ifdef USE_SPORT  
SoftwareSerial softwareSerial;
Smartport sport( &Serial );
#endif

unsigned long lastStats = millis();

//=====================================================================
//=====================================================================
void processIncomingTelemetry()
{
  while ( hxrcTelemetrySerial.getAvailable() > 0 && externalBTSerial.availableForWrite() > 0)
  {
    uint8_t c = hxrcTelemetrySerial.read();
    externalBTSerial.write( c );
  }
}

//=====================================================================
//=====================================================================
void fillOutgoingTelemetry()
{
  
  while ( (externalBTSerial.available() > 0) && (hxrcTelemetrySerial.getAvailableForWrite() > 0) )
  {
    uint8_t c = externalBTSerial.read();
    //Serial.print(char(c));
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


#ifdef USE_SPORT
//=====================================================================
//=====================================================================
int log_vprintf(const char *fmt, va_list args)
{
  char buffer[256];
  vsnprintf(buffer, 256, fmt, args);
  buffer[255] = 0;
  softwareSerial.print (buffer);
  return strlen(buffer);
}
#endif

unsigned char reverse(unsigned char b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

void test()
{
  pinMode( 1, OUTPUT );
  digitalWrite(1,HIGH);

  SPI.begin(-1, -1, 1, -1);

  SPI.setFrequency(115200);
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  SPI.setHwCs(false);


  uint32_t c = reverse('D');
  c= (c << 8);
  c|=0xffff00fe;

  SPI.write32(c);  
  SPI.write32(c);  
  SPI.write32(c);  
  SPI.write32(c);  


/*
  SPI.beginTransaction(SPISettings( 115200, MSBFIRST, SPI_MODE0 ));
  SPI.write32(c);  
  SPI.endTransaction();
  *

  SPI.beginTransaction(SPISettings( 115200, MSBFIRST, SPI_MODE0 ));
  SPI.write32(0xffffffff);  
  SPI.endTransaction();
  */
  
}


//=====================================================================
//=====================================================================
void setup()
{
#ifdef USE_SPORT  
  sport.init();

  softwareSerial.begin( 115200, SWSERIAL_8N1, -1, CP2102_RX_PIN );
  softwareSerial.enableIntTx(true);
  //esp_log_level_set("*", ESP_LOG_DEBUG);    
  esp_log_set_vprintf(&log_vprintf);
  HXRCSetLogStream( &softwareSerial );
#else
  Serial.begin(115200, SERIAL_8N1);  
  pinMode(SPORT_PIN,INPUT);
#endif

//test();

  HXRCLOG.println("Start");

  initLedPin();

  sbusDecoder.init(USE_SERIAL1_RX_PIN);

  setLed(false);

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
  hxrcMaster.updateLed( LED_PIN, false );

/*
  if (millis() - lastStats > 1000)
  {
    lastStats = millis();
    hxrcMaster.getTransmitterStats().printStats();
    hxrcMaster.getReceiverStats().printStats();
    if ( sbusDecoder.isFailsafe()) HXRCLOG.print("SBUS FS!\n");
  }*/

#ifdef USE_SPORT  
  sport.setRSSI( hxrcMaster.getTransmitterStats().getRSSI());
  sport.loop();
#endif
 }


