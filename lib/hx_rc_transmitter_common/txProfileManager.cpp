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
}

//=====================================================================
//=====================================================================
void TXProfileManager::loadConfig(int profileIndex)
{
    char fname[32];
    sprintf( fname, "/profile%d.json", profileIndex+1 );
    Serial.println(fname);
    File file = SPIFFS.open(fname);

    if ( !file) 
    {
        if ( profileIndex != CONFIG_PROFILE_INDEX) 
        {
            ErrorLog::instance.write("Unable to open ");
            ErrorLog::instance.write(fname);
            ErrorLog::instance.write("\n");
        }

        deserializeJson(this->json, configProfile);

        return;
    }

    DeserializationError error = deserializeJson(this->json, file);
    file.close();

    if (error)
    {
        ErrorLog::instance.write("Unable to parse ");
        ErrorLog::instance.write(fname);
        ErrorLog::instance.write(": ");
        ErrorLog::instance.write(error.c_str());
        ErrorLog::instance.write("\n");

        deserializeJson(this->json, configProfile);

        return;
    }
}

//=====================================================================
//=====================================================================
void TXProfileManager::loadConfigProfile()
{
    this->loadConfig(CONFIG_PROFILE_INDEX);
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
            this->loadConfig(this->currentProfileIndex);
        }
    }
}