#pragma once

#include <Arduino.h>

#include <stdio.h>
#include <stdarg.h>

#include "modeBase.h"

//=====================================================================
//=====================================================================
class ModeBLEGamepad : public ModeBase
{
private:

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


