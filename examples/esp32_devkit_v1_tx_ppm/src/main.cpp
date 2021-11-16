#include <Arduino.h>

#include <stdio.h>
#include <stdarg.h>

#include <esp_task_wdt.h>

#include "HX_ESPNOW_RC_Master.h"
#include "HX_ESPNOW_RC_SerialBuffer.h"
#include "nk_ppm_decoder.h"

#include "tx_config.h"
#include "HC06Interface.h"

#include "smartport.h"

#include "modeIdle.h"

#define WDT_TIMEOUT_SECONDS 3  

static NKPPMDecoder ppmDecoder;
static HC06Interface externalBTSerial(&Serial2);

#ifdef USE_SPORT  
SoftwareSerial softwareSerial;
Smartport sport( &Serial );
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

  TXConfigProfile::loadConfig();

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

  HXRCLOG.println("Start");

  initLedPin();

  ppmDecoder.init(USE_SERIAL1_RX_PIN);

  setLed(true);

  externalBTSerial.init();

  ModeBase::currentModeHandler = &ModeIdle::instance;
  ModeBase::currentModeHandler->start();

  esp_task_wdt_reset();
}

//=====================================================================
//=====================================================================
void loop()
{
  esp_task_wdt_reset();

  ppmDecoder.loop();

#ifdef USE_SPORT
  ModeBase::currentModeHandler->loop( &ppmDecoder, & externalBTSerial, &sport );
#else
  ModeBase::currentModeHandler->loop( &ppmDecoder, & externalBTSerial, NULL );
#endif  

}


