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
void ModeEspNowRC::processIncomingTelemetry(MavEsp8266Interface* MavEsp8266Serial, FrSkyTxTelemetryInterface* FrSkyTxSerial)
{
  while ( this->hxrcTelemetrySerial.getAvailable() > 0 && MavEsp8266Serial->availableForWrite() > 0)
  {
    uint8_t c = hxrcTelemetrySerial.read();
    MavEsp8266Serial->write( c );
  }
}

//=====================================================================
//=====================================================================
void ModeEspNowRC::fillOutgoingTelemetry(MavEsp8266Interface* MavEsp8266Serial)
{
  
  while ( (MavEsp8266Serial->available() > 0) && (hxrcTelemetrySerial.getAvailableForWrite() > 0) )
  {
    uint8_t c = MavEsp8266Serial->read();
    //Serial.print(char(c));
    hxrcTelemetrySerial.write(c);
  }
  
}


//=====================================================================
//=====================================================================
void ModeEspNowRC::setChannels( PPMDecoder* ppmDecoder )
{
    if (!ppmDecoder->isFailsafe())
    {
        //15 channels
        for ( int i = 0; i < HXRC_CHANNELS-1; i++ )
        {
        uint16_t r = ppmDecoder->getChannelValueInRange( i, 1000, 2000 );
        //if ( i == 3 ) Serial.println(r);
        hxrcMaster.setChannelValue( i, r );
        }
    }

    //use channel 16 to transmit failsafe flag
    hxrcMaster.setChannelValue( 15, ppmDecoder->isFailsafe() ? 1 : 0 );
}



//=====================================================================
//=====================================================================
void ModeEspNowRC::loop(
        PPMDecoder* ppmDecoder,
        MavEsp8266Interface* MavEsp8266Serial,
        Smartport* sport,
        FrSkyTxTelemetryInterface* FrSkyTxSerial
    )
{
    ModeBase::loop(ppmDecoder, MavEsp8266Serial, sport, FrSkyTxSerial);

    setChannels(ppmDecoder);

    hxrcTelemetrySerial.flush();
    processIncomingTelemetry(MavEsp8266Serial, FrSkyTxSerial);
    fillOutgoingTelemetry( MavEsp8266Serial);

    hxrcMaster.loop();
    hxrcMaster.updateLed( LED_PIN, true );  //LED_PIN is not inverted. Pass"inverted" flag so led is ON in idle mode

/*
  if (millis() - lastStats > 1000)
  {
    lastStats = millis();
    hxrcMaster.getTransmitterStats().printStats();
    hxrcMaster.getReceiverStats().printStats();
    if ( ppmDecoder.isFailsafe()) HXRCLOG.print("SBUS FS!\n");
  }
*/

#ifdef USE_SPORT  
    sport->setRSSI( hxrcMaster.getTransmitterStats().getRSSI());
    //sport->setA1(hxrcMaster.getA1());
    
    //removeme
    sport->setA1(capture.rate);

    sport->setA2(hxrcMaster.getA2());
    sport->setDIYValue(0, -capture.rssi );
    sport->setDIYValue(1, -capture.noiseFloor );
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
