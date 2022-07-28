#include <Arduino.h>
#include <ArduinoOTA.h>
#include <esp_task_wdt.h>

//include for HXRCLOG usage
#include "HX_ESPNOW_RC_Common.h"

#include "modeXiroMini.h"
#include "txProfileManager.h"

#include <WiFi.h>
#include <WiFiUdp.h>

#include <esp_wifi.h>
//#include <esp_wifi_internal.h>

#define UDP_CMD_PORT 7088
#define UDP_RTP_PORT 7078

ModeXiroMini ModeXiroMini::instance;
const char* ModeXiroMini::name = "XIROMINI";

static WiFiUDP udpCMD;
static WiFiUDP udpRTP;

//=====================================================================
//=====================================================================
ModeXiroMini::ModeXiroMini() 
{

}

//=====================================================================
//=====================================================================
static void ICACHE_RAM_ATTR sniffer_callback(void *buf, wifi_promiscuous_pkt_type_t type)
{
    if  (type != WIFI_PKT_MGMT) return;

    static const uint8_t ACTION_SUBTYPE = 0xd0;

    const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buf;
    const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
    const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;

    if ( 
        (ACTION_SUBTYPE == (hdr->frame_ctrl & 0xFF) ) &&
        (memcmp( hdr->addr2, capture.peerMac, 6 ) == 0 )  //mac is first 6 digits
    )
    {
        capture.rssi = ppkt->rx_ctrl.rssi;
        capture.noiseFloor = ppkt->rx_ctrl.noise_floor;
        capture.rate = ppkt->rx_ctrl.rate;
        capture.packetsCount++;
    }
}


//=====================================================================
//=====================================================================
void ModeXiroMini::start( JsonDocument* json )
{
    ModeBase::start(json);

    pinMode(LED_PIN,OUTPUT);
    digitalWrite(LED_PIN, HIGH );

    esp_task_wdt_reset();

    this->connected = false;

    JsonDocument* profile = TXProfileManager::instance.getCurrentProfile();

    WiFi.mode(WIFI_AP_STA);
    esp_wifi_set_protocol (WIFI_IF_STA, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N );
    esp_wifi_set_protocol (WIFI_IF_AP, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N );

    const IPAddress local(192, 168, 4, 1);
    const IPAddress dns(192,168,4,1);
    const IPAddress gateway(192,168,4,1); 
    const IPAddress netmask(255, 255, 255, 0);
    WiFi.softAPConfig(local, dns, netmask);

    //access point part
    WiFi.softAP( (*profile)["ap_name"] | "XIRO_RC", (*profile)["ap_password"] | "XIRO1234", (*profile)["ap_channel"] | 3, 0, 3 );
    HXRCLOG.print("AP IP:\n");
    HXRCLOG.println(WiFi.softAPIP());

    //station part
    WiFi.begin( (*profile)["drone_name"] | "XPLORER_Mini_0b4a41", (*profile)["drone_password"] | "XIRO1234");    

    delay(1000);

    //udpCMD.begin( UDP_CMD_PORT );
    //udpCMD.begin( IPAddress( 192,168,4,1), UDP_CMD_PORT );
    //udpRTP.begin( UDP_RTP_PORT );

    //esp_wifi_set_promiscuous_rx_cb(&sniffer_callback);
    //esp_wifi_set_promiscuous(true); 

    this->lastStats = millis();
    this->lastPacketTime = millis();
    this->lastRTPPacketTime = millis();
    this->lastRTPTrigger = millis();

    this->cmdPacketsSent = 0;
    this->cmdPacketsReceived = 0;
    this->rtpPacketsTotal = 0;
    this->cmdPacketsMirrored = 0;
    this->rtpPacketsMirrored = 0;

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
    udpCMD.write( value );
    *checksum += value; 
}

//=====================================================================
//=====================================================================
void ModeXiroMini::udpWriteSwitchWithChecksum( const HXChannels* channels, uint8_t channelIndex, uint16_t minValue, uint8_t* checksum )
{
    uint16_t r = channels->channelValue[channelIndex];

    if( r < 1250 ) r = minValue;
    else if ( r > 1750 ) r = 2000;
    else r = 1500;

    this->udpWriteWithChecksum( r >> 8, checksum);
    this->udpWriteWithChecksum( r & 0xff, checksum);
}

//=====================================================================
//=====================================================================
void ModeXiroMini::sendChannels( const HXChannels* channels )
{
    //Sending UDP on ESP32 in Station Mode
    //https://gist.github.com/santolucito/4016405f54850f7a216e9e453fe81803

    uint16_t r;

    udpCMD.beginPacket("192.168.1.1", UDP_CMD_PORT);

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
        r = map( channels->channelValue[i], 1000, 2000, 1100, 1900 );
        this->udpWriteWithChecksum( r >> 8, &checksum);
        this->udpWriteWithChecksum( r & 0xff, &checksum);
    }

	//5 - headless/ioc
	//Xiro app: 03E8			
	//FligtGo: 05dc
    //this->udpWriteWithChecksum( 0x03, &checksum);  
    //this->udpWriteWithChecksum( 0xE8, &checksum);
    this->udpWriteSwitchWithChecksum(  channels, 5-1, 0x3e8,  &checksum);

	//6 - rth?
	//Xiro App: 03e8
	//Flight Go: 00dc 
    //this->udpWriteWithChecksum( 0x03, &checksum);  
    //this->udpWriteWithChecksum( 0xE8, &checksum);
    this->udpWriteSwitchWithChecksum(  channels, 6-1, 0x00dc, &checksum);

	//7 - wheel r   PTZ  up+ dn-
	//Xiro App: 05dc
	//FlihgtGo: 05dc
    //this->udpWriteWithChecksum( 0x05, &checksum);  
    //this->udpWriteWithChecksum( 0xDC, &checksum);
    this->udpWriteSwitchWithChecksum(  channels, 7-1, 0x3e8,  &checksum);

	//8 - wheel l+  photo/video on/off
	//Xiro app: 05dc
	//Flightgo: 0003	
    //this->udpWriteWithChecksum( 0x05, &checksum);  
    //this->udpWriteWithChecksum( 0xDC, &checksum);

    this->udpWriteWithChecksum( 0x00, &checksum);  
    this->udpWriteWithChecksum( 0x03, &checksum);

    //this->udpWriteSwitchWithChecksum(  channels, 8-1, 0x0003,  &checksum);


	//9 - takeof landing?
	//Xiro app: 03e8
	//FlightGo: 05dc
    //this->udpWriteWithChecksum( 0x03, &checksum);   
    //this->udpWriteWithChecksum( 0xE8, &checksum);
    this->udpWriteSwitchWithChecksum(  channels, 9-1, 0x3e8, &checksum);

	//10 -  ???  rates? poshold?  mode 1 - 2000(slow) mode 2,3(fast) - 1500
	//Xiro app: 07dc
	//FlihgtGo: 05dc
    //this->udpWriteWithChecksum( 0x07, &checksum);  
    //this->udpWriteWithChecksum( 0xD0, &checksum);
    this->udpWriteSwitchWithChecksum(  channels, 10-1, 0x3e8, &checksum);

    udpCMD.write( checksum );

    udpCMD.endPacket();    

    this->cmdPacketsSent++;
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

    int packetSize = udpCMD.parsePacket();
    if (packetSize)
    {
        this->cmdPacketsReceived++;

        //if ( packetSize == 9 )
        {
            //HXRCLOG.printf("Received %d bytes from %s, to port %d\n", packetSize, udpCMD.remoteIP().toString().c_str(), UDP_CMD_PORT);
        }
        
        char incomingPacket[255];
        int len = udpCMD.read(incomingPacket, 255);
        if (len >= 98 && incomingPacket[0]== 0x0a && incomingPacket[1]== 0x62 && incomingPacket[2]== 0x01)
        {
            this->batPercentage = incomingPacket[52];
            this->numSats = incomingPacket[23];
            this->height = incomingPacket[47] + (((uint16_t)incomingPacket[48]) << 8 );
            //todo
        }

        udpCMD.beginPacket("192.168.4.2", UDP_CMD_PORT);
        udpCMD.write((const uint8_t*)incomingPacket, len);
        if ( udpCMD.endPacket() > 0 )
        {
            this->cmdPacketsMirrored++;
        }
    } 

    packetSize = udpRTP.parsePacket();
    if (packetSize)
    {
        this->rtpPacketsTotal++;

        //HXRCLOG.printf("Received %d bytes from %s, to port %d from port %d\n", packetSize, udpRTP.remoteIP().toString().c_str(), 7078, udpRTP.remotePort());
        
        lastRTPPacketTime = millis();

        char incomingPacket[1460];
        int len = udpRTP.read(incomingPacket, 1460);

        udpRTP.beginPacket("192.168.4.2", UDP_RTP_PORT);
        udpRTP.write((const uint8_t*)incomingPacket, len);
        if ( udpRTP.endPacket() > 0 )
        {
            this->rtpPacketsMirrored++;
        }

    } 
}

//=====================================================================
//=====================================================================
void ModeXiroMini::startVideo()
{
    udpCMD.beginPacket("192.168.1.1", UDP_CMD_PORT);
        udpCMD.write( 0xa0 );
        udpCMD.write( 0x09 );
        udpCMD.write( 0xe1 );
        udpCMD.write( 0x01 );
        udpCMD.write( 0x51 );
        udpCMD.write( 0x28 );
        udpCMD.write( 0xa6 );
        udpCMD.write( 0x1b );
        udpCMD.write( 0xc5 );
    udpCMD.endPacket();
}

//=====================================================================
//=====================================================================
void ModeXiroMini::loop(
        const HXChannels* channels,
        HC06Interface* externalBTSerial,
        Smartport* sport
    )
{
    ModeBase::loop(channels, externalBTSerial, sport);

    if (channels->isFailsafe ) 
    {
        digitalWrite(LED_PIN, LOW );
        if ( this->connected == true )
        {
            this->connected = false;
        }
    }
    else if ( (WiFi.status() != WL_CONNECTED) )
    {
        digitalWrite(LED_PIN, HIGH );
        if ( this->connected == true )
        {
            this->connected = false;
            udpCMD.stop();
            udpRTP.stop();
        }
    }
    else
    {
        digitalWrite(LED_PIN, ((millis() & 512) > 0 ) ? HIGH : LOW );
        if ( this->connected == false )
        {
            this->connected = true;
            udpCMD.begin( WiFi.localIP(), UDP_CMD_PORT );
            udpRTP.begin( UDP_RTP_PORT );
            HXRCLOG.println("Station IP:\n");      
            HXRCLOG.println(WiFi.localIP());      
        }

        if ((millis() - lastPacketTime) > 100)
        {
            lastPacketTime = millis();
            this->sendChannels(channels);
            this->fireDataflowEvent();
        }

        if ((millis() - lastRTPPacketTime) > 5000)
        {
            lastRTPPacketTime = millis();
            udpRTP.stop();
            udpRTP.begin( UDP_RTP_PORT );
        }

        if ((millis() - lastRTPTrigger) > 3000)
        {
            lastRTPTrigger = millis();
            this->startVideo();
        }

    }

    this->readPackets();

    processIncomingTelemetry(externalBTSerial);
    fillOutgoingTelemetry( externalBTSerial);

    if (millis() - lastStats > 3000)
    {
        lastStats = millis();
        if ( channels->isFailsafe) HXRCLOG.print("SBUS FS!\n");
        HXRCLOG.print(WiFi.status() == WL_CONNECTED ? "Connected " : "Connecting ");
        HXRCLOG.printf("cmdOut: %d cmdIn:%d/%d, RTP: %d/%d\n", 
        this->cmdPacketsSent, 
        this->cmdPacketsReceived, this->cmdPacketsMirrored, 
        this->rtpPacketsTotal, this->rtpPacketsMirrored );
        //channels->dump();
        //HXRCLOG.printf("RSSI:%d\n", WiFi.RSSI());
    }

    if ( sport != NULL )
    {
        sport->setRSSI( (WiFi.status() == WL_CONNECTED) ? -WiFi.RSSI(): 0 );
        sport->setA1( this->batPercentage );
        sport->setA2( this->numSats );
        sport->setAltitude( (uint32_t)this->height*100 );
        sport->loop();
    }

    if ( haveToChangeProfile() )
    {
        rebootToRequestedProfile();
    }
}
