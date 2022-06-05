#pragma once

#include <Arduino.h>

#include <stdio.h>
#include <stdarg.h>

#include "modeBase.h"

#define BLE_GAMEPAD_AXIS_COUNT 6
#define BLE_GAMEPAD_BUTTON_COUNT 9

//=====================================================================
//=====================================================================
class ModeBLEGamepad : public ModeBase
{
private:
    bool connected;

    int16_t axes[BLE_GAMEPAD_AXIS_COUNT];
    bool buttons[BLE_GAMEPAD_BUTTON_COUNT];
    void sendControls(const HXChannels* channels);

public:
    static ModeBLEGamepad instance;
    static const char* name;

    ModeBLEGamepad();

    void start( JsonDocument* json );

    void loop(
        const HXChannels* channels,
        HC06Interface* externalBTSerial,
        Smartport* sport
    );
};


