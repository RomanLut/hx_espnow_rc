#include <Arduino.h>
#include <ArduinoOTA.h>
#include <esp_task_wdt.h>

//include for HXRCLOG usage
#include "HX_ESPNOW_RC_Common.h"

#include "modeBLEGamepad.h"
#include "tx_config.h"

#include <WiFi.h>
#include <WiFiUdp.h>

#include <esp_wifi.h>
#include "esp_wifi_internal.h" 

ModeBLEGamepad ModeBLEGamepad::instance;

//=====================================================================
//=====================================================================
ModeBLEGamepad::ModeBLEGamepad() 
{

}

//=====================================================================
//=====================================================================
void ModeBLEGamepad::start()
{
    ModeBase::start();

    pinMode(LED_PIN,OUTPUT);
    digitalWrite(LED_PIN, HIGH );

    esp_task_wdt_reset();
}

//=====================================================================
//=====================================================================
void ModeBLEGamepad::loop(
        HXSBUSDecoder* sbusDecoder,
        HC06Interface* externalBTSerial,
        Smartport* sport
    )
{
    ModeBase::loop(sbusDecoder, externalBTSerial, sport);

    if (sbusDecoder->isFailsafe() ) 
    {
        digitalWrite(LED_PIN, LOW );
    }
    else
    {
        digitalWrite(LED_PIN, ((millis() & 512) > 0 ) ? HIGH : LOW );
    }

#ifdef USE_SPORT  
    sport->setRSSI( 100);
    sport->loop();
#endif

    if ( haveToChangeProfile() )
    {
        rebootToRequestedProfile();
    }
}
