#pragma once

#include <Arduino.h>

#include <stdio.h>
#include <stdarg.h>

#include <ESPxWebFlMgr.h>
#include <ESP-FTP-Server-Lib.h>

#include "modeBase.h"

//=====================================================================
//=====================================================================
class ModeConfig : public ModeBase
{
private:

    ESPxWebFlMgr filemgr;
    FTPServer ftp;

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


