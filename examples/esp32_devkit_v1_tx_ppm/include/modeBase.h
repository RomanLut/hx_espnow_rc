#pragma once

#include <Arduino.h>

#include <stdio.h>
#include <stdarg.h>

#include "smartport.h"
#include "HC06Interface.h"
#include "nk_ppm_decoder.h"

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
        NKPPMDecoder* ppmDecoder,
        HC06Interface* externalBTSerial,
        Smartport* sport
    );

    //got stable value from SBUS CH16 for profile selection?
    boolean haveStableSBUSProfileSelection();

    //current profile does not match profile requsted fro SBUS CH16?
    boolean haveToChangeProfile();

    void startRequestedProfile();
    void rebootToRequestedProfile();

};


