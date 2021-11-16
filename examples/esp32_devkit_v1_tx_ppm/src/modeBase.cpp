#include "modeBase.h"
#include "tx_config.h"

#include "HX_ESPNOW_RC_Master.h"

#include "modeEspNowRC.h"
#include "modeXiroMini.h"

ModeBase* ModeBase::currentModeHandler;

//=====================================================================
//=====================================================================
void ModeBase::start()
{
    this->gotSBUSProfileTime = 0;
    this->SBUSProfile = -1;

    HXRCLOG.println("Waiting for CH16 profile");
}

//=====================================================================
//=====================================================================
void ModeBase::loop(
        NKPPMDecoder* ppmDecoder,
        HC06Interface* externalBTSerial,
        Smartport* sport
)
{
    if ( !ppmDecoder->isFailsafe())
    {
        int s = getProfileIndexFromChannelValue(ppmDecoder->getChannelValueInRange( 15, 1000, 2000 ) );

        if ( this->SBUSProfile != s )
        {
            HXRCLOG.print("Got profile:");
            HXRCLOG.println(s);

            this->SBUSProfile = s;
            this->gotSBUSProfileTime = millis();
        }
    }
    else
    {
        this->gotSBUSProfileTime = 0;
    }

#ifdef USE_SPORT  
    sport->setDIYValue(2, TXConfigProfile::getCurrentProfileIndex()>=0? TXConfigProfile::getCurrentProfileIndex() : 255);
    sport->loop();
#endif

}

//=====================================================================
//=====================================================================
boolean ModeBase::haveStableSBUSProfileSelection()
{
    return (this->gotSBUSProfileTime!=0) && ((millis() - this->gotSBUSProfileTime)> 1000) && (this->SBUSProfile>=0);
}

//=====================================================================
//=====================================================================
boolean ModeBase::haveToChangeProfile()
{
    return this->haveStableSBUSProfileSelection() && (this->SBUSProfile != TXConfigProfile::getCurrentProfileIndex() );
}

//=====================================================================
//=====================================================================
void ModeBase::startRequestedProfile()
{
    HXRCLOG.print("Starting profile ");
    HXRCLOG.println( SBUSProfile );

    TXConfigProfile::setCurrentProfileIndex(SBUSProfile);

    switch( TXConfigProfile::getCurrentProfile()->transmitterMode )
    {
        case TM_ESPNOW:
            ModeBase::currentModeHandler = &ModeEspNowRC::instance;
            break;

        case TM_BLUETOOTH_GAMEPAD:
            //ModeBase::currentModeHandler = ModeEspNowRC::instance;
            break;

        case TM_XIRO_MINI:
            ModeBase::currentModeHandler = &ModeXiroMini::instance;
            break;

    }

    ModeBase::currentModeHandler->start();
}

//=====================================================================
//=====================================================================
void ModeBase::rebootToRequestedProfile()
{
    HXRCLOG.print( "Rebooting to profile" );
    HXRCLOG.println( SBUSProfile );
    delay(100);

    ESP.restart();
    delay(10000);
}

//=====================================================================
//=====================================================================
int ModeBase::getProfileIndexFromChannelValue( int value)
{
    //1000/PROFILES_COUNT = 100
    //500/PROFILES_COUNT = 50

    //1001 => 51 / 100 = 0
    //1049 => 99 / 100 = 0
    //1050 => 100 / 100 = 1
    //1100 => 150 / 100 = 1
    //1149 => 199 / 100 = 1
    //1200 => 250 / 100 = 2 
    return (int)((value - 1000 + (500/PROFILES_COUNT)) / (1000/PROFILES_COUNT));
}
