#pragma once

#include <Arduino.h>

#include <stdio.h>
#include <stdarg.h>

#include "smartport.h"
#include "HC06Interface.h"

#include "hx_channels.h"

//=====================================================================
//=====================================================================
class ModeBase
{
private:
    //0 - not getting any channels data (failsafe)
    //millis value - time since we are getting stable profile index from CH16 (decoded to CH16Profile)
    unsigned long gotCH16ProfileTime = 0; 
    
    int CH16Profile = -1; 

    unsigned long lastCycleTime = 0; 

    int getProfileIndexFromChannelValue( int value);

public:
    static ModeBase* currentModeHandler;

    virtual void start();

    virtual void loop(
        const HXChannels* channels,
        HC06Interface* externalBTSerial,
        Smartport* sport
    );

    //got stable value from CH16 for profile selection?
    boolean haveStableCH16ProfileSelection();

    //current profile does not match profile requested by CH16?
    boolean haveToChangeProfile();

    void startRequestedProfile();
    void rebootToRequestedProfile();

};


