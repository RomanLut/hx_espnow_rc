#include <Arduino.h>
#include <ArduinoOTA.h>
#include <esp_task_wdt.h>

//include for HXRCLOG usage
#include "HX_ESPNOW_RC_Common.h"

#include "modeE58.h"
#include "txProfileManager.h"

#include <WiFi.h>
#include <WiFiUdp.h>

#include <esp_wifi.h>
#include "esp_wifi_internal.h" 

#define UDP_CMD_PORT 50000

#define  CMD_NULL       0x00
#define  CMD_TAKE_OFF   0x01
#define  CMD_LAND       0x02
#define  CMD_EMERGENCY  0x04
#define  CMD_ROLL       0x08
#define  CMD_HEADLESS   0x10
#define  CMD_LOCK       0x20
#define  CMD_UNLOCK     0x40
#define  CMD_CALIBRATE  0x80

ModeE58 ModeE58::instance;
const char* ModeE58::name = "E58";

static WiFiUDP udpCMD;
static WiFiUDP udpRTP;

//=====================================================================
//=====================================================================
ModeE58::ModeE58() 
{

}

//=====================================================================
//=====================================================================
/*
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
*/

//=====================================================================
//=====================================================================
void ModeE58::start( JsonDocument* json )
{
    ModeBase::start(json);

    pinMode(LED_PIN,OUTPUT);
    digitalWrite(LED_PIN, HIGH );

    esp_task_wdt_reset();

    this->connected = false;

    WiFi.mode(WIFI_AP_STA);
    esp_wifi_set_protocol (WIFI_IF_STA, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N );
    esp_wifi_set_protocol (WIFI_IF_AP, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N );

    const IPAddress local(192, 168, 4, 1);
    const IPAddress dns(192,168,4,1);
    const IPAddress gateway(192,168,4,1); 
    const IPAddress netmask(255, 255, 255, 0);
    WiFi.softAPConfig(local, dns, netmask);

    //access point part
    WiFi.softAP("E58_RC","",3,0,3);
    HXRCLOG.print("AP IP:\n");
    HXRCLOG.println(WiFi.softAPIP());

    //station part
    WiFi.begin("WiFi-720P-2CDC1C", "");    

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
void ModeE58::processIncomingTelemetry(HC06Interface* externalBTSerial)
{
}

//=====================================================================
//=====================================================================
void ModeE58::fillOutgoingTelemetry(HC06Interface* externalBTSerial)
{
}

//=====================================================================
//=====================================================================
void ModeE58::sendCommand( byte x, byte y, byte z, byte rot, byte command )
{
    static byte msg[] = {0x66, x, y, z, rot, command, 0x00, 0x99};
    msg[6] = msg[1] ^ msg[2] ^ msg[3] ^ msg[4] ^ msg[5];

    udpCMD.beginPacket("192.168.0.1", UDP_CMD_PORT);
    udpCMD.write( msg, sizeof(msg) );
    udpCMD.endPacket();    
}

//=====================================================================
//=====================================================================
void ModeE58::sendChannels( const HXChannels* channels )
{
    //Sending UDP on ESP32 in Station Mode
    //https://gist.github.com/santolucito/4016405f54850f7a216e9e453fe81803

    //Hacking Chinese Drones for Fun and (No) Profit
    //https://blog.horner.tj/hacking-chinese-drones-for-fun-and-no-profit/

    /*
    1st byte – Header: 66
    2nd byte – Left/right movement (0-254, with 128 being neutral)
    3rd byte – Forward/backward movement (0-254, with 128 being neutral)
    4th byte – Throttle (elevation) (0-254, with 128 being neutral)
    5th byte – Turning movement (0-254, with 128 being neutral)
    6th byte – Reserved for commands (0 = no command)
    7th byte – Checksum (XOR of bytes 2, 3, 4, and 5)
    8th byte – Footer: 99
    */

    uint16_t r;

    //AETR
    this->sendCommand(
        map( channels->channelValue[0], 1000, 2000, 0, 254 ),
        map( channels->channelValue[1], 1000, 2000, 0, 254 ),
        map( channels->channelValue[2], 1000, 2000, 0, 254 ),
        map( channels->channelValue[3], 1000, 2000, 0, 254 ),
        CMD_NULL);

    this->cmdPacketsSent++;
}

//=====================================================================
//=====================================================================
void ModeE58::loop(
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
            HXRCLOG.println("Station IP:\n");      
            HXRCLOG.println(WiFi.localIP());      
        }

        if ((millis() - lastPacketTime) > 100)
        {
            lastPacketTime = millis();
            this->sendChannels(channels);
        }
    }

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
