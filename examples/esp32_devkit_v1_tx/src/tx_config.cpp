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

    //Hardcoded profile 1: hx_espnow_rc
    TXConfigProfile::profiles[0].espnow_channel = 3;
    TXConfigProfile::profiles[0].espnow_key = 0;
    TXConfigProfile::profiles[0].espnow_lrMode = false;
    TXConfigProfile::profiles[0].ap_name = "hxrct";

    //Hardcoded profile 2: hx_espnow_rc in LR mode
    TXConfigProfile::profiles[1].espnow_channel = 3;
    TXConfigProfile::profiles[1].espnow_key = 0;
    TXConfigProfile::profiles[1].espnow_lrMode = true;
    TXConfigProfile::profiles[1].ap_name = "hxrct1";

    //Hardcoded profile 3: Xiro mini
    TXConfigProfile::profiles[2].transmitterMode = TM_XIRO_MINI;

    //Hardcoded profile 4: BLE Gamepad
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