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

extern uint8_t currentProfileIndex;
extern HC06Interface externalBTSerial;

//=====================================================================
//=====================================================================
class TXMain
{
private:

    void initLedPin();
    void initLEDS4Pins();

public:
    static TXMain instance;

    void setLed( bool value );
    void setLEDS4(uint8_t v);

    void loadLastProfile();
    void saveLastProfile();

    void setup();
    void loop();
};

