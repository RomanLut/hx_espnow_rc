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
        HXSBUSDecoder* sbusDecoder,
        HC06Interface* externalBTSerial,
        Smartport* sport
    );
};


