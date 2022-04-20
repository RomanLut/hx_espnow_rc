#include "modeBase.h"
#include "txProfileManager.h"

#include "HX_ESPNOW_RC_Master.h"

#include "modeEspNowRC.h"
#include "modeXiroMini.h"
#include "modeBLEGamepad.h"

ModeBase* ModeBase::currentModeHandler;

//=====================================================================
//=====================================================================
void ModeBase::start()
{
    this->gotCH16ProfileTime = 0;
    this->CH16Profile = -1;

    HXRCLOG.println("Waiting for CH16 profile");
}

//=====================================================================
//=====================================================================
void ModeBase::loop(
        const HXChannels* channels,
        HC06Interface* externalBTSerial,
        Smartport* sport
)
{
    if ( !channels->isFailsafe)
    {
        int s = getProfileIndexFromChannelValue(channels->channelValue[15]  );

        if ( this->CH16Profile != s )
        {
            HXRCLOG.print("Got profile:");
            HXRCLOG.println(s);

            this->CH16Profile = s;
            this->gotCH16ProfileTime = millis();
        }
    }
    else
    {
        this->gotCH16ProfileTime = 0;
    }

    unsigned long t = millis();
    unsigned long dt = t - this->lastCycleTime;
    this->lastCycleTime = t;

    if ( sport != NULL )
    {
        sport->setR9PWR( 20 );
        sport->setProfileId( TXProfileManager::getCurrentProfileIndex()>=0? TXProfileManager::getCurrentProfileIndex() : 255 );
        sport->setDebug1( dt>5000? 5000 : 0 );

        sport->loop();
    }

}

//=====================================================================
//=====================================================================
boolean ModeBase::haveStableCH16ProfileSelection()
{
    return (this->gotCH16ProfileTime!=0) && ((millis() - this->gotCH16ProfileTime)> 1000) && (this->CH16Profile>=0);
}

//=====================================================================
//=====================================================================
boolean ModeBase::haveToChangeProfile()
{
    return this->haveStableCH16ProfileSelection() && (this->CH16Profile != TXProfileManager::getCurrentProfileIndex() );
}

//=====================================================================
//=====================================================================
void ModeBase::startRequestedProfile()
{
    HXRCLOG.print("Starting profile ");
    HXRCLOG.println( CH16Profile );

    TXProfileManager::setCurrentProfileIndex(CH16Profile);

    switch( TXProfileManager::getCurrentProfile()->transmitterMode )
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

        case TM_BLE_GAMEPAD:
            ModeBase::currentModeHandler = &ModeBLEGamepad::instance;
            break;

    }

    ModeBase::currentModeHandler->start();
}

//=====================================================================
//=====================================================================
void ModeBase::rebootToRequestedProfile()
{
    HXRCLOG.print( "Rebooting to profile" );
    HXRCLOG.println( CH16Profile );
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
