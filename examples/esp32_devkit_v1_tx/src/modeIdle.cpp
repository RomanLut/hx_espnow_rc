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
        HXSBUSDecoder* sbusDecoder,
        HC06Interface* externalBTSerial,
        Smartport* sport
)
{
    ModeBase::loop(sbusDecoder, externalBTSerial, sport);

    if ( haveToChangeProfile() )
    {
        startRequestedProfile();
    }
}
