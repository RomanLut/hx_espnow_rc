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

FTPServer ftp;

//=====================================================================
//=====================================================================
ModeConfig::ModeConfig() 
{
}

//=====================================================================
//=====================================================================
void ModeConfig::start()
{
    ModeBase::start();
    
    HXRCLOG.println("Config mode start\n");      

    pinMode(LED_PIN,OUTPUT);
    digitalWrite(LED_PIN, HIGH );

    esp_task_wdt_reset();

    if ( TXProfileManager::getCurrentProfile()->ap_name )
    {
        WiFi.softAP(
            TXProfileManager::getCurrentProfile()->ap_name, 
            TXProfileManager::getCurrentProfile()->ap_password, 
            TXProfileManager::getCurrentProfile()->espnow_channel);  //for ESPNOW RC mode, have to use channel configured from espnow rc

        ArduinoOTA.begin();  
    }

    esp_task_wdt_reset();

    if ( TXProfileManager::getCurrentProfile()->ap_name && TXProfileManager::getCurrentProfile()->ftp_user)
    {
        ftp.addUser(TXProfileManager::getCurrentProfile()->ftp_user, TXProfileManager::getCurrentProfile()->ftp_password);
        ftp.addFilesystem("SPIFFS", &SPIFFS);
        ftp.begin();  
    }

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

    if ( TXProfileManager::getCurrentProfile()->ap_name && TXProfileManager::getCurrentProfile()->ftp_user)
    {
        ftp.handle();
    }
}
