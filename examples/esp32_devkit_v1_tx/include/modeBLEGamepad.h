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

    int16_t axes[BLE_GAMEPAD_AXIS_COUNT];
    bool buttons[BLE_GAMEPAD_BUTTON_COUNT];
    void sendControls(HXSBUSDecoder* sbusDecoder);

public:
    static ModeBLEGamepad instance;

    ModeBLEGamepad();

    void start();

    void loop(
        HXSBUSDecoder* sbusDecoder,
        HC06Interface* externalBTSerial,
        Smartport* sport
    );
};


