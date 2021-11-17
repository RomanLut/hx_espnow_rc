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
        PPMDecoder* ppmDecoder,
        HC06Interface* externalBTSerial,
        Smartport* sport
)
{
    ModeBase::loop(ppmDecoder, externalBTSerial, sport);

    if ( haveToChangeProfile() )
    {
        startRequestedProfile();
    }
}
