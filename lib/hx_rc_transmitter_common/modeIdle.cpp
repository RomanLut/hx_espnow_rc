#include "modeIdle.h"

ModeIdle ModeIdle::instance;

//=====================================================================
//=====================================================================
void ModeIdle::start( JsonDocument* json )
{
    ModeBase::start(json);
}

//=====================================================================
//=====================================================================
void ModeIdle::loop(
        const HXChannels* channels,
        HC06Interface* externalBTSerial,
        Smartport* sport
)
{
    ModeBase::loop(channels, externalBTSerial, sport);

    if ( haveToChangeProfile() )
    {
        startRequestedProfile();
    }
}
