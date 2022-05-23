#pragma once

#include <Arduino.h>

#include <stdio.h>
#include <stdarg.h>

#include <esp_task_wdt.h>

#include <ArduinoJson.h>

#include "HX_ESPNOW_RC_Master.h"
#include "HX_ESPNOW_RC_SerialBuffer.h"

#include "txProfileManager.h"

#include "tx_config.h"
#include "HC06Interface.h"

#include "modeIdle.h"

#include "hx_channels.h"

#include <SPIFFS.h> 


//=====================================================================
//=====================================================================
class StateBase
{
private:
public:

    static StateBase* currentState;

    static void Goto(StateBase* s);

    virtual void onEnter(StateBase *prevState);
    virtual void onRun(uint32_t t);

    void SetLEDS4Profile(uint8_t profileIndex);
};

