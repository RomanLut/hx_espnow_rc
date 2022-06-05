#include <Arduino.h>
#include <ArduinoOTA.h>
#include <esp_task_wdt.h>

#include "modeEspNowRC.h"
#include "txProfileManager.h"


ModeEspNowRC ModeEspNowRC::instance;
const char* ModeEspNowRC::name = "ESPNOW";

//=====================================================================
//=====================================================================
ModeEspNowRC::ModeEspNowRC() : hxrcTelemetrySerial(&hxrcMaster)
{

}

//=====================================================================
//=====================================================================
void ModeEspNowRC::start( JsonDocument* json )
{
    ModeBase::start( json );

    JsonDocument* profile = TXProfileManager::instance.getCurrentProfile();

    this->LRMode = (*profile)["espnow_long_range_mode"] | false;

    this->hxrcMaster.init(
        HXRCConfig(
            (*profile)["espnow_channel"] | 3,
            (*profile)["espnow_key"] | 0,
            this->LRMode,
            -1, false));

    esp_task_wdt_reset();

    if ( (*profile)["ap_name"].as<const char*>() )
    {
        WiFi.softAP(
            (*profile)["ap_name"], 
            (*profile)["ap_password"] | "", 
            (*profile)["espnow_channel"] | 3);  //for ESPNOW RC mode, have to use channel configured for espnow rc

        ArduinoOTA.begin();  
    }

    esp_task_wdt_reset();

    this->lastStats = millis();
    this->lastFailsafe = true;
}


//=====================================================================
//=====================================================================
void ModeEspNowRC::processIncomingTelemetry(HC06Interface* externalBTSerial)
{
  while ( this->hxrcTelemetrySerial.getAvailable() > 0 && externalBTSerial->availableForWrite() > 0)
  {
    uint8_t c = hxrcTelemetrySerial.read();
    externalBTSerial->write( c );
  }
}

//=====================================================================
//=====================================================================
void ModeEspNowRC::fillOutgoingTelemetry(HC06Interface* externalBTSerial)
{
  
  while ( (externalBTSerial->available() > 0) && (hxrcTelemetrySerial.getAvailableForWrite() > 0) )
  {
    uint8_t c = externalBTSerial->read();
    //Serial.print(char(c));
    hxrcTelemetrySerial.write(c);
  }
  
}


//=====================================================================
//=====================================================================
void ModeEspNowRC::setChannels( const HXChannels* channels )
{
    if (!channels->isFailsafe)
    {
        //15 channels
        for ( int i = 0; i < HXRC_CHANNELS_COUNT-1; i++ )
        {
        uint16_t r = channels->channelValue[i];
        //if ( i == 3 ) Serial.println(r);
        hxrcMaster.setChannelValue( i, r );
        }
    }

    //use channel 16 to transmit failsafe flag
    hxrcMaster.setChannelValue( 15, channels->isFailsafe ? 1 : 0 );
}



//=====================================================================
//=====================================================================
void ModeEspNowRC::loop(
        const HXChannels* channels,
        HC06Interface* externalBTSerial,
        Smartport* sport
    )
{
    ModeBase::loop(channels, externalBTSerial, sport);

    setChannels(channels);

    hxrcTelemetrySerial.flushIn();
    processIncomingTelemetry(externalBTSerial);

    fillOutgoingTelemetry( externalBTSerial);
    hxrcTelemetrySerial.flushOut();

    hxrcMaster.loop();
    hxrcMaster.updateLed( LED_PIN, true );  //LED_PIN is not inverted. Pass"inverted" flag so led is ON in idle mode


  if (millis() - lastStats > 1000)
  {
    lastStats = millis();
    hxrcMaster.getTransmitterStats().printStats();
    hxrcMaster.getReceiverStats().printStats();
    if ( channels->isFailsafe) HXRCLOG.print("SBUS FS!\n");
  }

  if ( this->lastFailsafe != hxrcMaster.getReceiverStats().isFailsafe() )
  {
    this->lastFailsafe = hxrcMaster.getReceiverStats().isFailsafe();
    this->fire( this->lastFailsafe ? EVENT_DISCONNECTED : EVENT_CONNECTED );  
  } 

  if ( sport != NULL )
  {
    sport->setRSSI(hxrcMaster.getTransmitterStats().getRSSI());
    sport->setRSSIDbm( hxrcMaster.getTransmitterStats().getRSSIDbm() );
    sport->setNoiseFloor(hxrcMaster.getTransmitterStats().getNoiseFloor());
    sport->setSNR(hxrcMaster.getTransmitterStats().getSNR());

    sport->setRXRSSI(hxrcMaster.getReceiverStats().getRSSI());
    sport->setRXRSSIDbm( hxrcMaster.getReceiverStats().getRemoteRSSIDbm() );
    sport->setRXNoiseFloor(hxrcMaster.getReceiverStats().getRemoteNoiseFloor());
    sport->setRXSNR(hxrcMaster.getReceiverStats().getRemoteSNR());

    sport->setA1(hxrcMaster.getA1());
    sport->setA2(hxrcMaster.getA2());

    sport->setDebug2(hxrcMaster.getTransmitterStats().getRate()>=0? hxrcMaster.getTransmitterStats().getRate():255);
    sport->setDebug3(hxrcMaster.getTransmitterStats().getSuccessfulPacketRate());

    sport->loop();
  }

  if ( hxrcMaster.getReceiverStats().isFailsafe() && (*TXProfileManager::instance.getCurrentProfile())["ap_name"].as<const char*>() )
  {
      ArduinoOTA.handle();  
  }

  if ( !hxrcMaster.getReceiverStats().isFailsafe() )
  {
    this->fireDataflowEvent();
  }

  if ( haveToChangeProfile() )
  {
      rebootToRequestedProfile();
  }

}
