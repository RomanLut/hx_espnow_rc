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
        MavEsp8266Interface* MavEsp8266Serial,
        Smartport* sport,
        FrSkyTxTelemetryInterface* FrSkyTxSerial
)
{
    ModeBase::loop(ppmDecoder, MavEsp8266Serial, sport, FrSkyTxSerial);

    if ( haveToChangeProfile() )
    {
        startRequestedProfile();
    }
}
