#include "modeIdle.h"

ModeIdle ModeIdle::instance;

//=====================================================================
//=====================================================================
void ModeIdle::start()
{
    ModeBase::start();
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
