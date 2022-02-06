#include "smartport.h"
#include "tx_config.h"

TXConfigProfile TXConfigProfile::profiles[PROFILES_COUNT];
int TXConfigProfile::currentProfileIndex = -1;

//=====================================================================
//=====================================================================
TXConfigProfile::TXConfigProfile()
{
    this->transmitterMode = TM_ESPNOW;
    
    this->sportTelemetryEncoder = SPE_RSSI;
    
    this->espnow_key = 0;
    this->espnow_channel = 3;
    this->espnow_lrMode = false;

    this->ap_name = "hxrct";
    this->ap_password = NULL;
}


//=====================================================================
//=====================================================================
void TXConfigProfile::loadConfig()
{
    //TODO: load profiles from filesystem

    TXConfigProfile::profiles[1].espnow_lrMode = true;
    TXConfigProfile::profiles[1].ap_name = "hxrct1";

    TXConfigProfile::profiles[2].transmitterMode = TM_XIRO_MINI;

    TXConfigProfile::profiles[3].transmitterMode = TM_BLE_GAMEPAD;

}

//=====================================================================
//=====================================================================
const TXConfigProfile* TXConfigProfile::getCurrentProfile()
{
    return TXConfigProfile::currentProfileIndex < 0 ? NULL : &profiles[currentProfileIndex];
}

//=====================================================================
//=====================================================================
int TXConfigProfile::getCurrentProfileIndex()
{
    return TXConfigProfile::currentProfileIndex;
}

//=====================================================================
//=====================================================================
void TXConfigProfile::setCurrentProfileIndex(int index)
{
    TXConfigProfile::currentProfileIndex = index;
}