#pragma once

#include <Arduino.h>

#include <ArduinoJson.h>

#include <stdio.h>
#include <stdarg.h>

#include "smartport.h"
#include "HC06Interface.h"

#include "hx_channels.h"

#define EVENT_STARTUP           "STARTUP"

#define EVENT_CONNECTED         "CONNECTED"
#define EVENT_DISCONNECTED      "DISCONNECTED"

#define EVENT_TAKEOFF           "TAKEOFF"
#define EVENT_LANDING           "LANDING"
#define EVENT_EMERGENCY_STOP    "EMERGENCY_STOP"
#define EVENT_RTH               "RTH"

#define EVENT_HEADLESS_ENABLED  "HEADLESS_ENABLED"
#define EVENT_HEADLESS_DISABLED "HEADLESS_DISABLED"

#define EVENT_TRIM_ADJUST       "TRIM_ADJUST"
#define EVENT_TRIM_MAX          "TRIM_MAX"
#define EVENT_TRIM_CENTER       "TRIM_CENTER"

#define EVENT_GYRO_CALIBRATION  "GYRO_CALIBRATION"

#define EVENT_BEEP_SHORT        "BEEP_SHORT"


//=====================================================================
//=====================================================================
class ModeBase
{
private:
    //0 - not getting any channels data (failsafe)
    //millis value - time since we are getting stable profile index from CH16 (decoded to CH16Profile)
    unsigned long gotCH16ProfileTime = 0; 
    
    int CH16ProfileIndex = -1; 

    unsigned long lastCycleTime = 0; 

    int getProfileIndexFromChannelValue( int value);

public:
    static ModeBase* currentModeHandler;

    typedef void (*TModeEventHandler) (const char*); 
    static TModeEventHandler eventHandler;

    typedef void (*TDataflowEventHandler) (); 
    static TDataflowEventHandler eventDataFlowHandler;

    virtual void start( JsonDocument* json );

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

    void fire( const char* event );
    void fireDataflowEvent();

};


