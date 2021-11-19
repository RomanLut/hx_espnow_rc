#pragma once

#include <Arduino.h>

#include <stdio.h>
#include <stdarg.h>

#include "smartport.h"
#include "MavEsp8266Interface.h"
#include "FrSkyTxTelemetryInterface.h"
#include "ppm_decoder.h"

//=====================================================================
//=====================================================================
class ModeBase
{
private:
    //0 - not getting any sbus data
    //millis value - time since we are getting stable profile index from SBUS CH16 (decoded to SBUSProfile)
    unsigned long gotSBUSProfileTime = 0; 
    
    int SBUSProfile = -1; 

    int getProfileIndexFromChannelValue( int value);

public:
    static ModeBase* currentModeHandler;

    virtual void start();

    virtual void loop(
        PPMDecoder* ppmDecoder,
        MavEsp8266Interface* MavEsp8266Serial,
        Smartport* sport, 
        FrSkyTxTelemetryInterface* FrSkyTxSerial
        
    );

    //got stable value from SBUS CH16 for profile selection?
    boolean haveStableSBUSProfileSelection();

    //current profile does not match profile requsted fro SBUS CH16?
    boolean haveToChangeProfile();

    void startRequestedProfile();
    void rebootToRequestedProfile();

};


