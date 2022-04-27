#pragma once

#include <Arduino.h>
#include <Audio.h>

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

extern uint16_t CH16;
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

    void setup();
    void loop();
};

