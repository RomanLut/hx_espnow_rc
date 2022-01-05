#include <Arduino.h>
#include <ArduinoOTA.h>
#include <esp_task_wdt.h>

#include "modeEspNowRC.h"
#include "tx_config.h"


ModeEspNowRC ModeEspNowRC::instance;

//=====================================================================
//=====================================================================
ModeEspNowRC::ModeEspNowRC() : hxrcTelemetrySerial(&hxrcMaster)
{

}

//=====================================================================
//=====================================================================
void ModeEspNowRC::start()
{
    ModeBase::start();

    this->hxrcMaster.init(
        HXRCConfig(
            TXConfigProfile::getCurrentProfile()->espnow_channel,
            TXConfigProfile::getCurrentProfile()->espnow_key,
            TXConfigProfile::getCurrentProfile()->espnow_lrMode,
            -1, false));

    esp_task_wdt_reset();

    if ( TXConfigProfile::getCurrentProfile()->ap_name )
    {
        WiFi.softAP(
            TXConfigProfile::getCurrentProfile()->ap_name, 
            TXConfigProfile::getCurrentProfile()->ap_password, 
            TXConfigProfile::getCurrentProfile()->espnow_channel);  //for ESPNOW RC mode, have to use channel configured from espnow rc

        ArduinoOTA.begin();  
    }

    esp_task_wdt_reset();

    this->lastStats = millis();
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
void ModeEspNowRC::setChannels( HXSBUSDecoder* sbusDecoder )
{
    if (!sbusDecoder->isFailsafe())
    {
        //15 channels
        for ( int i = 0; i < HXRC_CHANNELS-1; i++ )
        {
        uint16_t r = sbusDecoder->getChannelValueInRange( i, 1000, 2000 );
        //if ( i == 3 ) Serial.println(r);
        hxrcMaster.setChannelValue( i, r );
        }
    }

    //use channel 16 to transmit failsafe flag
    hxrcMaster.setChannelValue( 15, sbusDecoder->isFailsafe() ? 1 : 0 );
}



//=====================================================================
//=====================================================================
void ModeEspNowRC::loop(
        HXSBUSDecoder* sbusDecoder,
        HC06Interface* externalBTSerial,
        Smartport* sport
    )
{
    ModeBase::loop(sbusDecoder, externalBTSerial, sport);

    setChannels(sbusDecoder);

    hxrcTelemetrySerial.flush();
    processIncomingTelemetry(externalBTSerial);
    fillOutgoingTelemetry( externalBTSerial);

    hxrcMaster.loop();
    hxrcMaster.updateLed( LED_PIN, true );  //LED_PIN is not inverted. Pass"inverted" flag so led is ON in idle mode

/*
  if (millis() - lastStats > 1000)
  {
    lastStats = millis();
    hxrcMaster.getTransmitterStats().printStats();
    hxrcMaster.getReceiverStats().printStats();
    if ( sbusDecoder.isFailsafe()) HXRCLOG.print("SBUS FS!\n");
  }
*/

#ifdef USE_SPORT  

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

    sport->loop();
#endif

    if ( hxrcMaster.getReceiverStats().isFailsafe() && TXConfigProfile::getCurrentProfile()->ap_name )
    {
        ArduinoOTA.handle();  
    }

    if ( haveToChangeProfile() )
    {
        rebootToRequestedProfile();
    }

}
