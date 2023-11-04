#include <Arduino.h>
#include <ArduinoOTA.h>
#include <esp_task_wdt.h>

#include "modeEspNowRC.h"
#include "txProfileManager.h"
#include "ErrorLog.h"

#include "HX_ESPNOW_RC_Slave.h"

ModeEspNowRC ModeEspNowRC::instance;
const char* ModeEspNowRC::name = "ESPNOW";

//=====================================================================
//=====================================================================
ModeEspNowRC::ModeEspNowRC() : hxrcTelemetrySerial(&hxrcMaster)
{

}

//=====================================================================
//=====================================================================
void ModeEspNowRC::start( JsonDocument* json, HC06Interface* externalBTSerial )
{
    ModeBase::start( json, externalBTSerial );

    JsonDocument* profile = TXProfileManager::instance.getCurrentProfile();

    HXRCConfig config(
            (*profile)["espnow_channel"] | 3,
            (*profile)["espnow_key"] | 0,
            this->LRMode,
            -1, false);

    String pktRate = (*profile)["packet_rate"] | "";
    if ( pktRate == "MAX" )
    {
        config.packetRatePeriodMS = HXRCConfig::PACKET_RATE_MAX;
    }
    else if ( pktRate != "" )
    {
        int pktRateInt = (*profile)["packet_rate"].as<int>() | 0;
        if ( pktRateInt > 0 )
        {
            config.packetRatePeriodMS = 1000 / pktRateInt;
        }
        else
        {
            ErrorLog::instance.write("Invalid packet rate:");
            ErrorLog::instance.write(pktRate.c_str());
            ErrorLog::instance.write("\n");
        }
    }

    String phyRate = (*profile)["phy_rate"] | "";
    if ( phyRate == "250K" )
    {
        config.wifiPhyRate = WIFI_PHY_RATE_LORA_250K;
    }
    else if ( phyRate == "500K" )
    {
        config.wifiPhyRate = WIFI_PHY_RATE_LORA_500K;
    }
    else if ( phyRate == "1M" )
    {
        config.wifiPhyRate = WIFI_PHY_RATE_1M_L;
    }
    else if ( phyRate == "2M" )
    {
        config.wifiPhyRate = WIFI_PHY_RATE_2M_L;
    }
    else if ( phyRate == "5M" )
    {
        config.wifiPhyRate = WIFI_PHY_RATE_5M_L;
    }
    else if ( phyRate == "11M" )
    {
        config.wifiPhyRate = WIFI_PHY_RATE_11M_L;
    }
    else
    {
        ErrorLog::instance.write("Invalid phy rate:");
        ErrorLog::instance.write(phyRate.c_str());
        ErrorLog::instance.write("\n");
    }

    this->LRMode = (*profile)["espnow_long_range_mode"] | false;

    this->hxrcMaster.init(config);

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


int ci = 0;
const char* m = "QUICK FOX JUMPS OVER THE LAZY DOG\n";

//=====================================================================
//=====================================================================
void ModeEspNowRC::processIncomingTelemetry(HC06Interface* externalBTSerial)
{
/*
      uint8_t c = hxrcTelemetrySerial.read();
      Serial.write( m[ci] );
      ci++;
      if ( m[ci] == 0 ) ci = 0;
*/      

  uint8_t buffer[256];

  if ( USBSerialTelemetryOutput )
  {
    //we do not call this->hxrcTelemetrySerial.getAvailable() as flushIn() is called explicitly
    
    int avOut = Serial.availableForWrite();

    while ( avOut > 0)
    {
      int count = hxrcTelemetrySerial.readUpTo( buffer, avOut > 256 ? 256 : avOut );
      if ( count == 0 ) break;
/*
  for ( int i =0; i < count; i++ )
  {
    buffer[i] = ci++;
  }
*/
/*
      static uint8_t b=0;
      for ( int i=0; i < count; i++ )
      {
          if ( buffer[i] != b++ )
          {
              b = buffer[i] + 1;
              hxrcMaster.getReceiverStats().onInvalidPacket();
          }
      }
*/
      Serial.write( buffer, count );


      avOut = Serial.availableForWrite();
    }
  }
  else
  {
    while ( this->hxrcTelemetrySerial.getAvailable() > 0 && externalBTSerial->availableForWrite() > 0)
    {
      uint8_t c = hxrcTelemetrySerial.read();
      externalBTSerial->write( c );
    }
  }


/*
  while ( externalBTSerial->availableForWrite() > 0)
  {
    externalBTSerial->write(m[ci]);
    ci++;
    if ( m[ci]==0) ci=0;
  }
*/
}

//=====================================================================
//=====================================================================
void ModeEspNowRC::fillOutgoingTelemetry(HC06Interface* externalBTSerial)
{
  while ( (externalBTSerial->available() > 0) && (hxrcTelemetrySerial.getAvailableForWrite() > 0) )
  {
    uint8_t c = externalBTSerial->read();
    //HXRCLOG.print(char(c));
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
          //if ( i == 3 ) HXRCLOG.println(r);
          hxrcMaster.setChannelValue( i, r );
        }
    }

    //use channel 16 to transmit failsafe flag (SBUS signal lost)
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
//int t1 = millis();

  ModeBase::loop(channels, externalBTSerial, sport);

  setChannels(channels);

  hxrcTelemetrySerial.flushIn();


  processIncomingTelemetry(externalBTSerial);  //2ms 


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

/*
t1 = millis() - t1;
if ( t1 > 1 )
{
    HXRCLOG.print("!Time:");
    HXRCLOG.println(t1);
}
*/
}
