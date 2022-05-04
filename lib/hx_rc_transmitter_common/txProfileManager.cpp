#include "txProfileManager.h"

#include <SPIFFS.h> 

#include "smartport.h"
#include "errorLog.h"

TXProfileManager TXProfileManager::instance;  

static const char* configProfile = "{\"transmitter_mode\" : \"CONFIG\", \"ap_name\" : \"hxrct\", \"ap_password\" : \"\", \"ftp_user\" : \"anonymous\", \"ftp_password\" : \"anonymous\" }";

//=====================================================================
//=====================================================================
TXProfileManager::TXProfileManager() : json(JSON_BUFFER_SIZE)
{
    this->currentProfileIndex = -1;

    /*
    this->transmitterMode = TM_ESPNOW;
    
    this->sportTelemetryEncoder = SPE_RSSI;
    
    this->espnow_key = 0;
    this->espnow_channel = 3;
    this->espnow_lrMode = false;

    this->ap_name = "hxrct";
    this->ap_password = NULL;
    */
}

//=====================================================================
//=====================================================================
void TXProfileManager::loadConfig()
{
    char fname[32];
    sprintf( fname, "/profile%d.json", this->currentProfileIndex+1 );
    Serial.println(fname);
    File file = SPIFFS.open(fname);

    if ( !file ) 
    {
        this->loadConfigProfile();

        ErrorLog::instance.write("Unable to open ");
        ErrorLog::instance.write(fname);
        ErrorLog::instance.write("\n");
        return;
    }

    DeserializationError error = deserializeJson(this->json, file);
    file.close();

    if (error)
    {
        this->loadConfigProfile();

        ErrorLog::instance.write("Unable to parse ");
        ErrorLog::instance.write(fname);
        ErrorLog::instance.write(": ");
        ErrorLog::instance.write(error.c_str());
        ErrorLog::instance.write("\n");
        return;
    }

    /*
    //TODO: load profiles from filesystem

    //Hardcoded profile 1: hx_espnow_rc
    TXProfileManager::profiles[0].espnow_channel = 3;
    TXProfileManager::profiles[0].espnow_key = 0;
    TXProfileManager::profiles[0].espnow_lrMode = false;
    TXProfileManager::profiles[0].ap_name = "hxrct";

    //Hardcoded profile 2: hx_espnow_rc in LR mode
    TXProfileManager::profiles[1].espnow_channel = 3;
    TXProfileManager::profiles[1].espnow_key = 0;
    TXProfileManager::profiles[1].espnow_lrMode = true;
    TXProfileManager::profiles[1].ap_name = "hxrct1";

    //Hardcoded profile 3: Xiro mini
    TXProfileManager::profiles[2].transmitterMode = TM_XIRO_MINI;

    //Hardcoded profile 4: BLE Gamepad
    TXProfileManager::profiles[3].transmitterMode = TM_BLE_GAMEPAD;

    //Hardcoded profile 5: Eachine E58
    TXProfileManager::profiles[4].transmitterMode = TM_E58;

    //Hardcoded profile 11: Config
    TXProfileManager::profiles[10].transmitterMode = TM_CONFIG;
    TXProfileManager::profiles[10].ap_name = "hxrct";
    TXProfileManager::profiles[10].espnow_channel = 3;
    TXProfileManager::profiles[10].ftp_user = "anonymous";
    TXProfileManager::profiles[10].ftp_password = "anonymous";
    */
}

//=====================================================================
//=====================================================================
void TXProfileManager::loadConfigProfile()
{
    deserializeJson(this->json, configProfile);
}

//=====================================================================
//=====================================================================
JsonDocument* TXProfileManager::getCurrentProfile()
{
    return this->currentProfileIndex < 0 ? NULL : &json;
}

//=====================================================================
//=====================================================================
int TXProfileManager::getCurrentProfileIndex()
{
    return this->currentProfileIndex;
}

//=====================================================================
//=====================================================================
void TXProfileManager::setCurrentProfileIndex(int index)
{
    if ( this->currentProfileIndex != index )
    {
        this->currentProfileIndex = index;
        if ( this->currentProfileIndex >= 0 )
        {
            this->loadConfig();
        }
    }
}