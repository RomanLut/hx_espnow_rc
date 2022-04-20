#include <Arduino.h>

#include <stdio.h>
#include <stdarg.h>

#include <esp_task_wdt.h>

#include "HX_ESPNOW_RC_Master.h"
#include "HX_ESPNOW_RC_SerialBuffer.h"
#include "hx_sbus_decoder.h"

#include "txProfileManager.h"

#include "tx_config.h"
#include "HC06Interface.h"

#include "smartport.h"

#include "modeIdle.h"

#include "hx_channels.h"

#define WDT_TIMEOUT_SECONDS 3  

static HXSBUSDecoder sbusDecoder;
static HC06Interface externalBTSerial;

#ifdef USE_SPORT  
SoftwareSerial softwareSerial;
Smartport sport;
#endif

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

//=====================================================================
//=====================================================================
void onOTAprogress( uint a, uint b )  
{
  esp_task_wdt_reset();
}

//=====================================================================
//=====================================================================
void setup()
{
  esp_task_wdt_init(WDT_TIMEOUT_SECONDS, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch

  TXProfileManager::loadConfig();

#ifdef USE_SPORT  
  sport.init( &Serial, SPORT_PIN );

  softwareSerial.begin( 115200, SWSERIAL_8N1, -1, CP2102_RX_PIN );
  softwareSerial.enableIntTx(true);
  //esp_log_level_set("*", ESP_LOG_DEBUG);    
  esp_log_set_vprintf(&log_vprintf);
  HXRCSetLogStream( &softwareSerial );
#else
  Serial.begin(115200, SERIAL_8N1);  
  pinMode(SPORT_PIN,INPUT);
#endif

  HXRCLOG.println("Start");

  initLedPin();

  sbusDecoder.init(USE_SERIAL1_RX_PIN);

  setLed(true);

  externalBTSerial.init(&Serial2, HC06_INTERFACE_RX_PIN, HC06_INTERFACE_TX_PIN);

  ModeBase::currentModeHandler = &ModeIdle::instance;
  ModeBase::currentModeHandler->start();

  esp_task_wdt_reset();
}

//=====================================================================
//=====================================================================
void  getChannelValues( HXSBUSDecoder* sbusDecoder, HXChannels* channelValues )
{
  channelValues-> isFailsafe = sbusDecoder->isFailsafe();
  for ( int i = 0; i < HXRC_CHANNELS_COUNT; i++)
  {
    channelValues->channelValue[i] = sbusDecoder->getChannelValueInRange( i, 1000, 2000);
  }

}

//=====================================================================
//=====================================================================
void loop()
{
  esp_task_wdt_reset();

  sbusDecoder.loop();

  HXChannels channelValues;
  getChannelValues( &sbusDecoder, &channelValues );


#ifdef USE_SPORT
  ModeBase::currentModeHandler->loop( &channelValues, & externalBTSerial, &sport );
#else
  ModeBase::currentModeHandler->loop( &channelValues, & externalBTSerial, NULL );
#endif  

}


