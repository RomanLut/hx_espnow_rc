#include <Arduino.h>
#include <ArduinoOTA.h>
#include <esp_task_wdt.h>

//include for HXRCLOG usage
#include "HX_ESPNOW_RC_Common.h"

#include "modeKYFPV.h"
#include "txProfileManager.h"

#include <WiFi.h>
#include <WiFiUdp.h>

#include <esp_wifi.h>
//#include <esp_wifi_internal.h> 

#define STATE_DISCONNECTED   0 
#define STATE_CONNECTED_WIFI 1 
#define STATE_CONNECTED      2

#define UDP_CMD_PORT    8800
#define UDP_CMD_IP      "192.168.169.1"

ModeKYFPV ModeKYFPV::instance;
const char* ModeKYFPV::name = "KYFPV";

static const byte START_PACKET[] = {0xef, 0x00, 0x04, 0x00};

//=====================================================================
//=====================================================================
ModeKYFPV::ModeKYFPV() 
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
void ModeKYFPV::start( JsonDocument* json )
{
    ModeBase::start(json);

    pinMode(LED_PIN,OUTPUT);
    digitalWrite(LED_PIN, HIGH );

    esp_task_wdt_reset();

    WiFi.mode(WIFI_STA);
    esp_wifi_set_protocol (WIFI_IF_STA, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N );

    //esp_wifi_set_promiscuous_rx_cb(&sniffer_callback);
    //esp_wifi_set_promiscuous(true); 

    this->lastPacketTime = millis();
    this->packetsCount = 0;
    this->state = STATE_DISCONNECTED;
    this->retryConnectionTime = millis();

    this->takeOffCount = 0;
    this->landingCount = 0;
    this->stopCount = 0;
    this->GyroCount = 0;

    this->headless = false;

    this->flipBeep = millis();
}


//=====================================================================
//=====================================================================
void ModeKYFPV::processIncomingTelemetry(HC06Interface* externalBTSerial)
{
}

//=====================================================================
//=====================================================================
void ModeKYFPV::fillOutgoingTelemetry(HC06Interface* externalBTSerial)
{
}

//=====================================================================
//=====================================================================
void ModeKYFPV::processButton( int channelValue, uint8_t* state, const char* event)
{
    if ( channelValue >= 1900 ) 
    {
        if ( (*state) == 0)
        {
            this->fire(event);
            (*state) = 1;
        }
    }
    else
    {
        if ( (*state) > 50 )
        {     
            (*state) = 0;
        }
    }

    if ( (*state) > 0 && (*state) < 100 )
    {
        (*state)++;
    }
}

//=====================================================================
//=====================================================================
void ModeKYFPV::sendBeginPacket()
{
    udpCMD.beginPacket(UDP_CMD_IP, UDP_CMD_PORT);
    udpCMD.write( START_PACKET, sizeof(START_PACKET) );
    udpCMD.endPacket();    
}

//=====================================================================
//=====================================================================
void ModeKYFPV::sendChannels( const HXChannels* channels )
{
    //Sending UDP on ESP32 in Station Mode
    //https://gist.github.com/santolucito/4016405f54850f7a216e9e453fe81803

    //Hacking Chinese Drones for Fun and (No) Profit
    //https://blog.horner.tj/hacking-chinese-drones-for-fun-and-no-profit/

    //ESP8266 UDP Send & Receive Example (Step-by-Step Tutorial)
    //https://siytek.com/esp8266-udp-send-receive/

    static byte msg[] = 
        { 
            0xef, 0x02, 0x28,                   //0 [ ] [pkt type?] [size]
            0x00, 0x02, 0x02, 0x00, 0x01,       //3 
            0x00,                               //8 [flags?]
            0x00, 0x00, 0x00,                   //9
            0x2a, 0x08,                         //12 packet count lo,hi
            0x00, 0x00,                         //14
            0x66,                               //16 pkt begin signature
            0x80, 0x80, 0x80, 0x80,             //17 roll pitch throlle yaw
            0x40,                               //21 command ?      44-stop, 42-landing, 41-takeoff
            0xc0,                               //22 xor
            0x99,                               //23 packet end signature
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };


/*
    static byte msg[] = 
        { 
            0xef, 0x02, 0x60,                   //0 [ ] [pkt type?] [size]
            0x00, 0x02, 0x02, 0x00, 0x01,       //3 
            0x03,                               //8 [flags?] 
            0x00, 0x00, 0x00,                   //9
            0x2a, 0x08,                         //12 packet count lo,hi
            0x00, 0x00,                         //14
            0x66,                               //16 pkt begin signature
            0x80, 0x80, 0x80, 0x80,             //17 roll pitch throlle yaw
            0x40,                               //21 command ?      44-stop, 42-landing, 41-takeoff
            0xc0,                               //22 xor
            0x99,                               //23 packet end signature
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //24
            0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,                           //40 
            0x88, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00,   //52
            0xff, 0xff, 0xff, 0xff, 0x8a, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,   //68
            0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00                            //84 
        };
*/
    msg[12] = this->packetsCount & 255;
    msg[13] = (this->packetsCount >> 8) & 255;
    msg[14] = (this->packetsCount >> 16) & 255;
    msg[15] = (this->packetsCount >> 24) & 255;

/*
    msg[52] = this->packetsCount & 255;
    msg[53] = (this->packetsCount >> 8) & 255;
    msg[54] = (this->packetsCount >> 16) & 255;
    msg[55] = (this->packetsCount >> 24) & 255;

    msg[72] = this->packetsCount & 255;
    msg[73] = (this->packetsCount >> 8) & 255;
    msg[74] = (this->packetsCount >> 16) & 255;
    msg[75] = (this->packetsCount >> 24) & 255;
*/

    //AETR  = Roll Pitch Throttle Yaw
    msg[17] = map( channels->channelValue[0], 1000, 2000, 2, 254 );
    msg[18] = map( channels->channelValue[1], 1000, 2000, 2, 254 );
    msg[19] = map( channels->channelValue[2], 1000, 2000, 2, 254 );   //0..255 - not althold, 0x80 - althold
    msg[20] = map( channels->channelValue[3], 1000, 2000, 2, 254 );
    
    msg[22] = msg[17] ^ msg[18] ^ msg[19] ^ msg[20] ^ msg[21];

    this->processButton( channels->channelValue[4], &this->takeOffCount, EVENT_TAKEOFF );
    this->processButton( channels->channelValue[5], &this->landingCount, EVENT_LANDING );
    this->processButton( channels->channelValue[6], &this->stopCount, EVENT_EMERGENCY_STOP );
    this->processButton( channels->channelValue[9], &this->GyroCount, EVENT_GYRO_CALIBRATION );

    if ( channels->channelValue[7] > 1750 )
    {
        if ( !this->headless )
        {
            this->headless = true;
            this->fire(EVENT_HEADLESS_ENABLED);
        }
    }
    else
    {
        if ( this->headless )
        {
            this->headless = false;
            this->fire(EVENT_HEADLESS_DISABLED);
        }
    }

    uint32_t t = millis();


    if ( channels->channelValue[8] > 1750 )
    {
        if ( t > this->flipBeep ) 
        {
            this->flipBeep = t + 1000;
            this->fire(EVENT_BEEP_SHORT);
        }
    }

    msg[21] = 0x40 //ARMED
    | ( (this->takeOffCount > 0 && this->takeOffCount <= 22 ) ? 1 : 0 ) 
    | ( (this->landingCount > 0 && this->landingCount <= 22 ) ? 2 : 0 )
    | ( (this->stopCount > 0 && this->stopCount <= 22 ) ? 4 : 0 ) 
    | ( channels->channelValue[8] > 1750 ? 8 : 0 ) //flip
    | ( channels->channelValue[7] > 1750 ? 16 : 0 ) //headless
    | ( (this->GyroCount > 0 && this->GyroCount <= 22 ) ? 128 : 0 );

    //Serial.println(msg[21]);

    udpCMD.beginPacket(UDP_CMD_IP, UDP_CMD_PORT);
    udpCMD.write( msg, sizeof(msg) );
    udpCMD.endPacket();    

    this->packetsCount++;

    //Serial.println(sizeof(msg));
}

//=====================================================================
//=====================================================================
void ModeKYFPV::loop(
        const HXChannels* channels,
        HC06Interface* externalBTSerial,
        Smartport* sport
    )
{
    ModeBase::loop(channels, externalBTSerial, sport);

    unsigned long t = millis();

    if ( (WiFi.status() != WL_CONNECTED) )
    {
        digitalWrite(LED_PIN, HIGH );

        if ( this->state != STATE_DISCONNECTED )
        {
            this->state = STATE_DISCONNECTED;

            WiFi.disconnect();
            udpCMD.stop();

            this->fire(EVENT_DISCONNECTED);
        }

        if ( this->retryConnectionTime <= t )
        {
            this->retryConnectionTime = t + 10000;

            if ( WiFi.status() != WL_IDLE_STATUS && WiFi.status() != WL_NO_SSID_AVAIL )  
            {
                Serial.println(WiFi.status());
                Serial.println("Connect...");
                WiFi.begin("KY-wifi-FD0B0B4A", "");    
            }
        }
    }
    else
    {
        if ( this->state == STATE_DISCONNECTED)
        {
            this->state = STATE_CONNECTED_WIFI;
        
            Serial.println("Connected to wifi...");
            udpCMD.begin( 40000 );

            this->retryConnectionTime = t;
        }

        if ( this->state == STATE_CONNECTED_WIFI)
        {
            if ( this->retryConnectionTime + 100 < t )
            {
                this->retryConnectionTime  = t + 100;
                this->sendBeginPacket();
            }
        }
        else
        {
            digitalWrite(LED_PIN, ((millis() & 512) > 0 ) ? HIGH : LOW );

            if ((millis() - lastPacketTime) >= 50)
            {
                lastPacketTime = millis();
                this->sendChannels(channels);
                Serial.print("*");
                this->fireDataflowEvent();
            }
        }
    }

    if ( this->state != STATE_DISCONNECTED )
    {
        int packetSize = udpCMD.parsePacket();
        if (packetSize)
        {

            if ( this->state == STATE_CONNECTED_WIFI )
            {
                this->fire(EVENT_CONNECTED);
            }

            this->state = STATE_CONNECTED;

            Serial.printf("Received %d bytes from %s:%d\n", packetSize, udpCMD.remoteIP().toString().c_str(), udpCMD.remotePort());
            
            char incomingPacket[1460];
            int len = udpCMD.read(incomingPacket, 1460);
        } 
    }

    processIncomingTelemetry(externalBTSerial);
    fillOutgoingTelemetry( externalBTSerial);

    if ( sport != NULL )
    {
        sport->setRSSI( (WiFi.status() == WL_CONNECTED) ? -WiFi.RSSI(): 0 );
        sport->loop();
    }

    if ( haveToChangeProfile() )
    {
        rebootToRequestedProfile();
    }
}

/* 

host:
192.168.169.1

port: 8800

0)  ef 00 04 00
initial packet

 
1) 
                                           ef 02 3c 00   ......"`.DI...<.
0040   02 02 00 01 01 00 00 00 39 05 00 00 66 80 80 00   ........9...f...
0050   80 40 c0 99 00 00 00 00 00 00 00 00 66 08 00 00   .@..........f...
0060   00 00 00 00 01 00 00 00 14 00 00 00 ff ff ff ff   ................
0070   00 00 00 00 00 00 00 00 

2) 
                                           ef 02 28 00   ......"`.0....(.
0040   02 02 00 01 00 00 00 00 2a 08 00 00 66 80 80 00   ........*...f...
0050   80 40 c0 99 00 00 00 00 00 00 00 00 00 00 00 00   .@..............
0060   00 00 00 00 

3)
                                           ef 02 60 00   ......"`.h....`.
0040   02 02 00 01 03 00 00 00 d8 2c 00 00 66 80 80 80   .........,..f...
0050   80 40 40 99 00 00 00 00 00 00 00 00 89 44 00 00   .@@..........D..
0060   00 00 00 00 01 00 00 00 14 00 00 00 ff ff ff ff   ................
0070   88 44 00 00 00 00 00 00 01 00 00 00 14 00 00 00   .D..............
0080   ff ff ff ff 8a 44 00 00 00 00 00 00 03 00 00 00   .....D..........
0090   10 00 00 00 00 00 00 00 00 00 00 00 


header ef 02
total_length   3c
00
02 
02
00
01
flags - depend on packet size
00
00
00
packet count lo
paket count hi
00
00
66
rol 80 mid
pitch 80 mid
throtte 0...255
yaw 80 mid
FLY_FLAGS 40 
rol ^ pitch ^ throttle ^ yaw ^ FLY_FLAGS
99
00
...
00


replay from port 1234:

                                                 93 01 
0040   7f 01 02 02 00 01 38 09 00 00 00 00 00 00|38|09| - pkt id
0050   00 00 00 00 00 00 36 09 00 00 00 00 00 00 11 00 

            int i = FlyController.this.isFastFly ? 1 : 0;
            if (FlyController.this.isFastDrop) {
                i += 2;
            }
            if (FlyController.this.isEmergencyStop) {
                i += 4;
            }
            if (FlyController.this.isCircleTurnEnd) {
                i += 8;
            }
            if (FlyController.this.isNoHeadMode) {
                i += 16;
            }
            if (FlyController.this.isFastReturn || FlyController.this.isUnLock) {
                i += 32;
            }
            if (FlyController.this.isOpenLight) {
                i += 64;
            }
            if (FlyController.this.isGyroCorrection) {
                i += 128;
            }
            if (FlyController.this.controlTurn > 255) {
                int unused = FlyController.this.controlTurn = 255;
            } else if (FlyController.this.controlTurn < 1) {
                int unused2 = FlyController.this.controlTurn = 1;
            }
            if (FlyController.this.controlAccelerator == 1) {
                int unused3 = FlyController.this.controlAccelerator = 0;
            }
            if (FlyController.this.controlByte1 > 255) {
                int unused4 = FlyController.this.controlByte1 = 255;
            } else if (FlyController.this.controlByte1 < 1) {
                int unused5 = FlyController.this.controlByte1 = 1;
            }
            if (FlyController.this.controlByte2 > 255) {
                int unused6 = FlyController.this.controlByte2 = 255;
            } else if (FlyController.this.controlByte2 < 1) {
                int unused7 = FlyController.this.controlByte2 = 1;
            }
            int access$1200 = (((FlyController.this.controlByte1 ^ FlyController.this.controlByte2) ^ FlyController.this.controlAccelerator) ^ FlyController.this.controlTurn) ^ (i & 255);
            if (FlyController.this.type == 2) {
                FlyCommandUtils.tryToSendFlyCommand(new int[]{102, FlyController.this.controlByte1, FlyController.this.controlByte2, FlyController.this.controlAccelerator, FlyController.this.controlTurn, i, access$1200, 153});
            } else {
                StreamClient.getInstance().sendCommand(new byte[]{102, (byte) FlyController.this.controlByte1, (byte) FlyController.this.controlByte2, (byte) FlyController.this.controlAccelerator, (byte) FlyController.this.controlTurn, (byte) i, (byte) access$1200, -103}, 0);
            }


    private final int SEND_COMMAND_INTERVAL = 50;
    private final int COMMAND_LENGTH = 8;

    public static final String CMD_FLYING_CTRL = "FLYING_CTRL";

    public static void tryToSendFlyCommand(int[] iArr) {
        if (deviceClient.isConnected()) {
            SettingCmd settingCmd = new SettingCmd();
            settingCmd.setTopic(FlyCommand.CMD_FLYING_CTRL);
            settingCmd.setOperation(Operation.TYPE_PUT);
            ArrayMap arrayMap = new ArrayMap();
            arrayMap.put("BYTE0", String.valueOf(iArr[0]));
            arrayMap.put("BYTE1", String.valueOf(iArr[1]));
            arrayMap.put("BYTE2", String.valueOf(iArr[2]));
            arrayMap.put("BYTE3", String.valueOf(iArr[3]));
            arrayMap.put("BYTE4", String.valueOf(iArr[4]));
            arrayMap.put("BYTE5", String.valueOf(iArr[5]));
            arrayMap.put("BYTE6", String.valueOf(iArr[6]));
            arrayMap.put("BYTE7", String.valueOf(iArr[7]));
            String str = tag;
            Dbug.i(str, "byte0:" + iArr[0] + ",byte1:" + iArr[1] + ",byte2:" + iArr[2] + ",byte3:" + iArr[3] + ",byte4:" + iArr[4] + ",byte5:" + iArr[5] + ",byte6:" + iArr[6] + ",byte7:" + iArr[7]);
            settingCmd.setParams(arrayMap);
            deviceClient.tryToPut(settingCmd, new SendResponse() {
                public void onResponse(Integer num) {
                    if (num.intValue() != 1) {
                        Dbug.e(FlyCommandUtils.tag, "SEND FLY COMMAND FAIL!");
                    }
                }
            });
        }
    }




*/

