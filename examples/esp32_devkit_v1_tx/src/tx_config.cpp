#include "Smartport.h"
#include "tx_config.h"

TXConfigProfile currentProfile;


//=====================================================================
//=====================================================================
void initConfig()
{
    //TODO: load profiles from filesystem
    //TODO: select current profile from CH16

    currentProfile.transmitterMode = TM_ESPNOW;
    
    currentProfile.sportTelemetryEncoder = SPE_RSSI;
    
    currentProfile.espnow_key = 0;
    currentProfile.espnow_channel = 3;
    currentProfile.espnow_lrMode = false;

    currentProfile.ap_name = "hxrct";
    currentProfile.ap_password = NULL;

}

