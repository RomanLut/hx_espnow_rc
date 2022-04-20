#pragma once
#include "modeBase.h"

//=====================================================================
//=====================================================================
class ModeIdle : public ModeBase
{
private:
public:
    static ModeIdle instance;

    void start();

    void loop(
        const HXChannels* channels,
        HC06Interface* externalBTSerial,
        Smartport* sport
    );
};


