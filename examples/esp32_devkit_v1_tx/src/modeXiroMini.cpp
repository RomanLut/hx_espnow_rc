#include <Arduino.h>
#include <ArduinoOTA.h>
#include <esp_task_wdt.h>

//include for HXRCLOG usage
#include "HX_ESPNOW_RC_COMMON.h"

#include "modeXiroMini.h"
#include "tx_config.h"

#include <WiFi.h>
#include <WiFiUdp.h>

#define UDP_PORT 7088

ModeXiroMini ModeXiroMini::instance;

static WiFiUDP udp;

//=====================================================================
//=====================================================================
ModeXiroMini::ModeXiroMini() 
{

}

//=====================================================================
//=====================================================================
void ModeXiroMini::start()
{
    ModeBase::start();

    pinMode(LED_PIN,OUTPUT);
    digitalWrite(LED_PIN, HIGH );

    esp_task_wdt_reset();

    this->connected = false;

    WiFi.mode(WIFI_STA);
    esp_wifi_set_protocol (WIFI_IF_STA, WIFI_PROTOCOL_11B );

    WiFi.begin("XPLORER_Mini_0b4a41", "XIRO1234");    

    this->lastStats = millis();
    this->lastPacketTime = millis();

    this->batPercentage = 0;
    this->height = 0;
    this->numSats = 0;
}


//=====================================================================
//=====================================================================
void ModeXiroMini::processIncomingTelemetry(HC06Interface* externalBTSerial)
{
}

//=====================================================================
//=====================================================================
void ModeXiroMini::fillOutgoingTelemetry(HC06Interface* externalBTSerial)
{
}

//=====================================================================
//=====================================================================
void ModeXiroMini::udpWriteWithChecksum( uint8_t value, uint8_t* checksum )
{
    udp.write( value );
    *checksum += value; 
}

//=====================================================================
//=====================================================================
void ModeXiroMini::udpWriteSwitchWithChecksum( HXSBUSDecoder* sbusDecoder, uint8_t channelIndex, uint8_t* checksum )
{
    uint16_t r = sbusDecoder->getChannelValueInRange( channelIndex , 1000, 2000 );

    if( r < 1250 ) r = 1000;
    else if ( r > 1750 ) r = 2000;
    else r = 1500;

    this->udpWriteWithChecksum( r >> 8, checksum);
    this->udpWriteWithChecksum( r & 0xff, checksum);
}

//=====================================================================
//=====================================================================
void ModeXiroMini::sendChannels( HXSBUSDecoder* sbusDecoder )
{
    //Sending UDP on ESP32 in Station Mode
    //https://gist.github.com/santolucito/4016405f54850f7a216e9e453fe81803

    uint16_t r;

    udp.beginPacket("192.168.1.1", UDP_PORT);

    uint8_t checksum = 0;

    this->udpWriteWithChecksum(0xA0, &checksum);
    this->udpWriteWithChecksum(0x18, &checksum); //frame_len
    this->udpWriteWithChecksum(0x01, &checksum);

    //AETR
    //A right+
    //E down+   set weight to -100
    //T down+   set weight to -100
    //R right+
    for ( int i = 0; i < 4; i++ )
    {
        r = sbusDecoder->getChannelValueInRange( i, 1100, 1900 );
        this->udpWriteWithChecksum( r >> 8, &checksum);
        this->udpWriteWithChecksum( r & 0xff, &checksum);
    }

    //this->udpWriteWithChecksum( 0x03, &checksum);  //5 - ???  headless? ioc?
    //this->udpWriteWithChecksum( 0xE8, &checksum);
    this->udpWriteSwitchWithChecksum(  sbusDecoder, 5-1,  &checksum);

    //this->udpWriteWithChecksum( 0x03, &checksum);  //6 - rth?
    //this->udpWriteWithChecksum( 0xE8, &checksum);
    this->udpWriteSwitchWithChecksum(  sbusDecoder, 6-1,  &checksum);

    //this->udpWriteWithChecksum( 0x05, &checksum);  //7 - wheel r   PTZ  up+ dn-
    //this->udpWriteWithChecksum( 0xDC, &checksum);
    this->udpWriteSwitchWithChecksum(  sbusDecoder, 7-1,  &checksum);

    //this->udpWriteWithChecksum( 0x05, &checksum);  //8 - wheel l+  photo/video on/off
    //this->udpWriteWithChecksum( 0xDC, &checksum);
    this->udpWriteSwitchWithChecksum(  sbusDecoder, 8-1,  &checksum);

    //this->udpWriteWithChecksum( 0x03, &checksum);   //9 - takeof landing?
    //this->udpWriteWithChecksum( 0xE8, &checksum);
    this->udpWriteSwitchWithChecksum(  sbusDecoder, 9-1,  &checksum);

    //this->udpWriteWithChecksum( 0x07, &checksum);  //10 -  ???  rates? poshold?  mode 1 - 2000(slow) mode 2,3(fast) - 1500
    //this->udpWriteWithChecksum( 0xD0, &checksum);
    this->udpWriteSwitchWithChecksum(  sbusDecoder, 10-1,  &checksum);

    udp.write( checksum );

    udp.endPacket();    
}

//=====================================================================
//=====================================================================
void ModeXiroMini::readPackets()
{
/*
        this.uavLat = C3265bk.m8937a(C3265bk.m8947a(bArr, 3, 4));
        this.uavLng = C3265bk.m8937a(C3265bk.m8947a(bArr, 7, 4));
        this.dstLng = C3265bk.m8937a(C3265bk.m8947a(bArr, 11, 4));
        this.dstLat = C3265bk.m8937a(C3265bk.m8947a(bArr, 15, 4));
        this.direction = (C3265bk.m8937a(C3265bk.m8947a(bArr, 19, 4)) * 180.0f) / 3.141f;
        this.gpsNumber = C3265bk.m8940a(C3265bk.m8947a(bArr, 23, 1), 1);
        this.year = C3265bk.m8940a(C3265bk.m8947a(bArr, 24, 1), 1);
        this.month = C3265bk.m8940a(C3265bk.m8947a(bArr, 25, 1), 1);
        this.day = C3265bk.m8940a(C3265bk.m8947a(bArr, 26, 1), 1);
        this.hour = C3265bk.m8940a(C3265bk.m8947a(bArr, 27, 1), 1);
        this.minute = C3265bk.m8940a(C3265bk.m8947a(bArr, 28, 1), 1);
        this.second = C3265bk.m8940a(C3265bk.m8947a(bArr, 29, 1), 1);
        this.wayPointsNumber = C3265bk.m8940a(C3265bk.m8947a(bArr, 30, 1), 1);
        this.manuallyRudder = C3265bk.m8940a(C3265bk.m8947a(bArr, 31, 1), 1);
        this.manuallyAileron = C3265bk.m8940a(C3265bk.m8947a(bArr, 32, 1), 1);
        this.manuallyElevator = C3265bk.m8940a(C3265bk.m8947a(bArr, 33, 1), 1);
        this.manuallyAccelerator = C3265bk.m8940a(C3265bk.m8947a(bArr, 34, 1), 1);
        this.realRudder = C3265bk.m8940a(C3265bk.m8947a(bArr, 35, 1), 1);
        this.realAileron = C3265bk.m8940a(C3265bk.m8947a(bArr, 36, 1), 1);
        this.realElevator = C3265bk.m8940a(C3265bk.m8947a(bArr, 37, 1), 1);
        this.realAccelerator = C3265bk.m8940a(C3265bk.m8947a(bArr, 38, 1), 1);
        this.speedY = C3265bk.m8949b(C3265bk.m8947a(bArr, 39, 2), 2);
        this.bootTime = C3265bk.m8940a(C3265bk.m8947a(bArr, 41, 2), 2);
        this.reserved1 = C3265bk.m8940a(C3265bk.m8947a(bArr, 43, 1), 1);
        this.reserved2 = C3265bk.m8940a(C3265bk.m8947a(bArr, 44, 1), 1);
        this.distanceFromStartPointHighBit = C3265bk.m8940a(C3265bk.m8947a(bArr, 45, 1), 1);
        this.radiusPtzToPoint = C3265bk.m8940a(C3265bk.m8947a(bArr, 46, 1), 1);
        this.height = C3265bk.m8962g(C3265bk.m8947a(bArr, 47, 2));
        this.speedGpsX = C3265bk.m8962g(C3265bk.m8947a(bArr, 49, 2));
        this.distanceFromStartPointLowBit = C3265bk.m8940a(C3265bk.m8947a(bArr, 51, 1), 1);
        this.batteryPercentage = C3265bk.m8940a(C3265bk.m8947a(bArr, 52, 1), 1);
        this.shakingCoefficient = C3265bk.m8940a(C3265bk.m8947a(bArr, 53, 1), 1);
        this.flyGearsMode = C3265bk.m8940a(C3265bk.m8947a(bArr, 54, 1), 1);
        this.shockCoefficient = C3265bk.m8940a(C3265bk.m8947a(bArr, 55, 1), 1);
        this.temperature = C3265bk.m8940a(C3265bk.m8947a(bArr, 56, 1), 1);
        this.rightAcceleration = C3265bk.m8962g(C3265bk.m8947a(bArr, 57, 2));
        this.backAcceleration = C3265bk.m8962g(C3265bk.m8947a(bArr, 59, 2));
        this.pitchAngle = C3265bk.m8940a(C3265bk.m8947a(bArr, 61, 4), 4);
        this.rollAngle = C3265bk.m8940a(C3265bk.m8947a(bArr, 65, 4), 4);
        this.flyControlVoltage = C3265bk.m8940a(C3265bk.m8947a(bArr, 69, 2), 2);
        this.downAcceleration = C3265bk.m8962g(C3265bk.m8947a(bArr, 71, 2));
        this.currentTaskId = C3265bk.m8940a(C3265bk.m8947a(bArr, 73, 1), 1);
        this.currentFlyMode = C3265bk.m8940a(C3265bk.m8947a(bArr, 74, 1), 1);
        this.flyTotalTime = C3265bk.m8940a(C3265bk.m8947a(bArr, 75, 2), 2);
        this.warningFlag = C3265bk.m8940a(C3265bk.m8947a(bArr, 77, 1), 1);
        this.speedDHeightBit = C3265bk.m8940a(C3265bk.m8947a(bArr, 78, 1), 1);
        this.medianRudder = C3265bk.m8940a(C3265bk.m8947a(bArr, 79, 1), 1);
        this.medianAileron = C3265bk.m8940a(C3265bk.m8947a(bArr, 80, 1), 1);
        this.medianElevator = C3265bk.m8940a(C3265bk.m8947a(bArr, 81, 1), 1);
        this.speedXHighBit = C3265bk.m8940a(C3265bk.m8947a(bArr, 82, 1), 1);
        this.dstHeight = C3265bk.m8962g(C3265bk.m8947a(bArr, 83, 2));
        this.reserved3 = C3265bk.m8940a(C3265bk.m8947a(bArr, 85, 3), 3);
        this.speedDLowBit = C3265bk.m8940a(C3265bk.m8947a(bArr, 88, 1), 1);
        this.flyTime = C3265bk.m8940a(C3265bk.m8947a(bArr, 89, 3), 3);
        this.ptzStatus = C3265bk.m8940a(C3265bk.m8947a(bArr, 91, 1), 1);
        this.speedXLowBit = C3265bk.m8940a(C3265bk.m8947a(bArr, 92, 1), 1);
        this.speedGpsY = C3265bk.m8962g(C3265bk.m8947a(bArr, 93, 2));
        this.version = C3265bk.m8940a(C3265bk.m8947a(bArr, 95, 2), 2);
*/

    int packetSize = udp.parsePacket();
    if (packetSize)
    {
        //HXRCLOG.printf("Received %d bytes from %s, port %d\n", packetSize, udp.remoteIP().toString().c_str(), udp.remotePort());
        
        char incomingPacket[255];
        int len = udp.read(incomingPacket, 255);
        if (len >= 98 && incomingPacket[0]== 0x0a && incomingPacket[1]== 0x62 && incomingPacket[2]== 0x01)
        {
            this->batPercentage = incomingPacket[52];
            this->numSats = incomingPacket[23];
            this->height = incomingPacket[47] + (((uint16_t)incomingPacket[48]) << 8 );
            //todo
        }
    } 
}

//=====================================================================
//=====================================================================
void ModeXiroMini::loop(
        HXSBUSDecoder* sbusDecoder,
        HC06Interface* externalBTSerial,
        Smartport* sport
    )
{
    ModeBase::loop(sbusDecoder, externalBTSerial, sport);

    if (sbusDecoder->isFailsafe() ) 
    {
        digitalWrite(LED_PIN, LOW );
        if ( this->connected == true )
        {
            udp.stop();
            this->connected = false;
        }
    }
    else if ( (WiFi.status() != WL_CONNECTED) )
    {
        digitalWrite(LED_PIN, HIGH );
        if ( this->connected == true )
        {
            udp.stop();
            this->connected = false;
        }
    }
    else
    {
        digitalWrite(LED_PIN, ((millis() & 512) > 0 ) ? HIGH : LOW );
        if ( this->connected == false )
        {
            this->connected = true;
            udp.begin( UDP_PORT );
        }

        this->sendChannels(sbusDecoder);

        this->readPackets();
    }

    processIncomingTelemetry(externalBTSerial);
    fillOutgoingTelemetry( externalBTSerial);

    if (millis() - lastStats > 1000)
    {
        lastStats = millis();
        if ( sbusDecoder->isFailsafe()) HXRCLOG.print("SBUS FS!\n");
        HXRCLOG.println(WiFi.status() == WL_CONNECTED ? "Connected" : "Connecting");
        //sbusDecoder->dump();
        //HXRCLOG.printf("RSSI:%d\n", WiFi.RSSI());
    }

#ifdef USE_SPORT  
    sport->setRSSI( (WiFi.status() == WL_CONNECTED) ? -WiFi.RSSI(): 0 );
    sport->setA1( this->batPercentage );
    sport->setA2( this->numSats );
    sport->setDIYValue( 0, this->height );
    sport->loop();
#endif

    if ( haveToChangeProfile() )
    {
        rebootToRequestedProfile();
    }
}
