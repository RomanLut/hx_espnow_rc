#include "modeBase.h"
#include "txProfileManager.h"

#include "HX_ESPNOW_RC_Master.h"

#include "modeEspNowRC.h"
#include "modeXiroMini.h"
#include "modeBLEGamepad.h"
#include "modeKYFPV.h"
#include "modeConfig.h"

#include "errorLog.h"

ModeBase* ModeBase::currentModeHandler;

ModeBase::TModeEventHandler ModeBase::eventHandler = NULL;
ModeBase::TDataflowEventHandler ModeBase::eventDataFlowHandler = NULL;

//=====================================================================
//=====================================================================
void ModeBase::start(JsonDocument* json)
{
    this->gotCH16ProfileTime = 0;
    this->CH16ProfileIndex = -1;

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

        if ( this->CH16ProfileIndex != s )
        {
            HXRCLOG.print("Got profile:");
            HXRCLOG.println(s);

            this->CH16ProfileIndex = s;
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

    if ( dt > 10 )
    {
        Serial.print("!Cycle time:");
        Serial.println(dt);
    }


    if ( sport != NULL )
    {
        sport->setR9PWR( 20 );
        sport->setProfileId( TXProfileManager::instance.getCurrentProfileIndex()>=0? TXProfileManager::instance.getCurrentProfileIndex() : 255 );
        sport->setDebug1( dt>5000? 5000 : 0 );

        sport->loop();
    }

}

//=====================================================================
//=====================================================================
boolean ModeBase::haveStableCH16ProfileSelection()
{
    return (this->gotCH16ProfileTime!=0) && ((millis() - this->gotCH16ProfileTime)> 1000) && (this->CH16ProfileIndex>=0);
}

//=====================================================================
//=====================================================================
boolean ModeBase::haveToChangeProfile()
{
    return this->haveStableCH16ProfileSelection() && (this->CH16ProfileIndex != TXProfileManager::instance.getCurrentProfileIndex() );
}

//=====================================================================
//=====================================================================
void ModeBase::startRequestedProfile()
{
    HXRCLOG.print("Starting profile ");
    HXRCLOG.println( CH16ProfileIndex );

    TXProfileManager::instance.setCurrentProfileIndex(CH16ProfileIndex);

    JsonDocument* json = TXProfileManager::instance.getCurrentProfile();

    if ( json )
    {
        const char* modeName = (*json)["transmitter_mode"] | "";

        if ( strcmp( modeName, ModeConfig::name ) == 0)
        {
            ModeBase::currentModeHandler = &ModeConfig::instance;
        }
        else if ( strcmp( modeName, ModeEspNowRC::name ) == 0)
        {
            ModeBase::currentModeHandler = &ModeEspNowRC::instance;
        }
        else if ( strcmp( modeName, ModeBLEGamepad::name ) == 0)
        {
            ModeBase::currentModeHandler = &ModeBLEGamepad::instance;
        }
        else if ( strcmp( modeName, ModeXiroMini::name ) == 0)
        {
            ModeBase::currentModeHandler = &ModeXiroMini::instance;
        }
        else if ( strcmp( modeName, ModeKYFPV::name ) == 0)
        {
            ModeBase::currentModeHandler = &ModeKYFPV::instance;
        }
        else
        {
            ErrorLog::instance.write("Unknown mode: ");
            ErrorLog::instance.write(modeName);
            ErrorLog::instance.write("\n");
        }

        ModeBase::currentModeHandler->start(json);
    }

}

//=====================================================================
//=====================================================================
void ModeBase::rebootToRequestedProfile()
{
    HXRCLOG.print( "Rebooting to profile" );
    HXRCLOG.println( CH16ProfileIndex );
    delay(100);

    ESP.restart();
    delay(10000);
}

//=====================================================================
//=====================================================================
int ModeBase::getProfileIndexFromChannelValue( int value)
{
    //1001 => 51 / 100 = 0
    //1049 => 99 / 100 = 0
    //1050 => 100 / 100 = 1
    //1100 => 150 / 100 = 1
    //1149 => 199 / 100 = 1
    //1200 => 250 / 100 = 2 
    //...
    //1300 => 3
    //...
    //1900 => 9
    //2000 => 10
    return (int)((value - 1000 + 50) / 100);
}

//=====================================================================
//=====================================================================
void ModeBase::fire( const char* event )
{
    Serial.print("Event:");
    Serial.println(event);
    if (ModeBase::eventHandler)
    {
        ModeBase::eventHandler(event);
    }
}

//=====================================================================
//=====================================================================
void ModeBase::fireDataflowEvent()
{
    if (ModeBase::eventDataFlowHandler)
    {
        ModeBase::eventDataFlowHandler();
    }
}
