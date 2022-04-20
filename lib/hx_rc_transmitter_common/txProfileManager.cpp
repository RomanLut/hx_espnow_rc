#include "smartport.h"
#include "txProfileManager.h"

TXProfileManager TXProfileManager::profiles[PROFILES_COUNT];
int TXProfileManager::currentProfileIndex = -1;

//=====================================================================
//=====================================================================
TXProfileManager::TXProfileManager()
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
void TXProfileManager::loadConfig()
{
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
}

//=====================================================================
//=====================================================================
const TXProfileManager* TXProfileManager::getCurrentProfile()
{
    return TXProfileManager::currentProfileIndex < 0 ? NULL : &profiles[currentProfileIndex];
}

//=====================================================================
//=====================================================================
int TXProfileManager::getCurrentProfileIndex()
{
    return TXProfileManager::currentProfileIndex;
}

//=====================================================================
//=====================================================================
void TXProfileManager::setCurrentProfileIndex(int index)
{
    TXProfileManager::currentProfileIndex = index;
}