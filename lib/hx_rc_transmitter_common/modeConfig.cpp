#include <Arduino.h>
#include <ArduinoOTA.h>
#include <esp_task_wdt.h>

//include for HXRCLOG usage
#include "HX_ESPNOW_RC_Common.h"

#include "modeConfig.h"
#include "txProfileManager.h"

#include <SPIFFS.h> 
#include <ESP-FTP-Server-Lib.h>

ModeConfig ModeConfig::instance;
const char* ModeConfig::name = "CONFIG";

FTPServer ftp;

//=====================================================================
//=====================================================================
ModeConfig::ModeConfig() 
{
}

//=====================================================================
//=====================================================================
void ModeConfig::start( JsonDocument* json )
{
    ModeBase::start(json);
    
    HXRCLOG.println("Config mode start\n");      

    pinMode(LED_PIN,OUTPUT);
    digitalWrite(LED_PIN, HIGH );

    esp_task_wdt_reset();

    JsonDocument* profile = TXProfileManager::instance.getCurrentProfile();

    WiFi.softAP( (*profile)["ap_name"] | "hxrct",  (*profile)["ap_password"] | "");  

    esp_task_wdt_reset();

    ftp.addUser((*profile)["ftp_user"] | "anonymous", (*profile)["ftp_password"] | "anonymous");
    ftp.addFilesystem("SPIFFS", &SPIFFS);
    ftp.begin();  

    ArduinoOTA.begin();  

    esp_task_wdt_reset();
}

//=====================================================================
//=====================================================================
void ModeConfig::loop(
        const HXChannels* channels,
        HC06Interface* externalBTSerial,
        Smartport* sport
    )
{
    ModeBase::loop(channels, externalBTSerial, sport);

    digitalWrite(LED_PIN, ((millis() & 512) > 0 ) ? HIGH : LOW );

    if ( haveToChangeProfile() )
    {
        rebootToRequestedProfile();
    }

    JsonDocument* profile = TXProfileManager::instance.getCurrentProfile();

    ftp.handle();

    ArduinoOTA.handle();
}
