#pragma once
#include "modeBase.h"

//=====================================================================
//=====================================================================
class ModeIdle : public ModeBase
{
private:
public:
    static ModeIdle instance;
    static const char* name;

    void start( JsonDocument* json );

    void loop(
        const HXChannels* channels,
        HC06Interface* externalBTSerial,
        Smartport* sport
    );
};


