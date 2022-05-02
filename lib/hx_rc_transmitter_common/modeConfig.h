#pragma once

#include <Arduino.h>

#include <stdio.h>
#include <stdarg.h>

#include "modeBase.h"

//=====================================================================
//=====================================================================
class ModeConfig : public ModeBase
{
private:

public:
    static ModeConfig instance;
    static const char* name;

    ModeConfig();

    void start( JsonDocument* json );

    void loop(
        const HXChannels* channels,
        HC06Interface* externalBTSerial,
        Smartport* sport
    );
};


