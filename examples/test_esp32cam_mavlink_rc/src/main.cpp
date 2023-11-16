#include <Arduino.h>
#include "HX_ESPNOW_RC_Slave.h"
#include "rx_config.h"
#include "hx_mavlink_rc_encoder.h"
#include "hx_mavlink_parser.h"
#include "hx_mavlink_frame_sender.h"
#include "HX_ESPNOW_RC_SerialBuffer.h"

#include <esp_task_wdt.h>

#include <ArduinoOTA.h>

#include "frame.h"


#define WDT_TIMEOUT_SECONDS 3  

HXRCSlave hxrcSlave;
HXRCSerialBuffer<1024> hxrcTelemetrySerial( &hxrcSlave );
HXMavlinkRCEncoder hxMavlinkRCEncoder;
HXMavlinkParser outgoingMavlinkParser(true);
HXMavlinkFrameSender mavlinkFrameSender;

unsigned long lastStats = millis();

//=====================================================================
//=====================================================================
typedef enum
{
    RCV_GOT_CONNECTION_ONCE  = 0,
    RCV_NORMAL_MODE          = 1,
    RVC_WAITING_CONNECTION   = 2,
} RSState;

uint8_t state = RVC_WAITING_CONNECTION;
unsigned long startTime = millis();

HardwareSerial mavlinkSerial(2);

//=====================================================================
//=====================================================================
void processIncomingTelemetry()
{
  while ( hxrcTelemetrySerial.getAvailable() > 0 && mavlinkSerial.availableForWrite() > 0)
  {
    uint8_t c = hxrcTelemetrySerial.read();
    //TODO: support incoming telemetry
  }
}

//=====================================================================
//=====================================================================
void trySendFrame()
{
  hxrcTelemetrySerial.flushOut();

  mavlinkFrameSender.send( hxrcTelemetrySerial );

  if (mavlinkFrameSender.isEmpty())
  {
    mavlinkFrameSender.addFrame(frame, FRAME_DATA_SIZE);
  }
}

//=====================================================================
//=====================================================================
void fillOutgoingTelemetry()
{
  //keep free space (300b) in outgoing queue so any packet can be put in

  while ( (mavlinkSerial.available() > 0) && (hxrcTelemetrySerial.getAvailableForWrite() > 300) ) 
  {
    uint8_t c = mavlinkSerial.read();
    //hxrcTelemetrySerial.write(c);
    outgoingMavlinkParser.processByte( c );
    if (outgoingMavlinkParser.gotPacket() )
    {
      //output valid packet
      hxrcTelemetrySerial.writeBuffer(outgoingMavlinkParser.getPacketBuffer(), outgoingMavlinkParser.getPacketLength());
      trySendFrame();
    }
  }
  
  while ( hxrcTelemetrySerial.getAvailableForWrite() > 300 )
  {
    trySendFrame();
  }
}

//=====================================================================
//=====================================================================
void onOTAprogress( uint a, uint b )  
{
  esp_task_wdt_reset();
}

//=====================================================================
//=====================================================================
void setup()
{
  esp_task_wdt_init(WDT_TIMEOUT_SECONDS, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch

  Serial.begin(115200);

  pinMode( MAVLINK_TX_PIN, OUTPUT);
  mavlinkSerial.begin(TELEMETRY_BAUDRATE, SERIAL_8N1, MAVLINK_RX_PIN, MAVLINK_TX_PIN );

  hxMavlinkRCEncoder.init( MAVLINK_RC_PACKET_RATE_MS, USE_MAVLINK_V1 );

  HXRCConfig config (
          USE_WIFI_CHANNEL,
          USE_KEY,
          false, //LR mode
          -1, false);
  
  config.packetRatePeriodMS = HXRCConfig::PACKET_RATE_MAX;
  config.slaveTelemertyPayloadSize = HXRC_SLAVE_TELEMETRY_SIZE_MAX;
  config.wifiPhyRate =  WIFI_PHY_RATE_5M_L;

  hxrcSlave.init(config);

  //REVIEW: receiver does not work if AP is not initialized?
  WiFi.softAP("hxrcmavlink", NULL, USE_WIFI_CHANNEL);

  ArduinoOTA.onProgress(&onOTAprogress);
  ArduinoOTA.begin();  

  size_t freeHeap = ESP.getFreeHeap();
  Serial.print("Free heap memory: ");
  Serial.print(freeHeap);
  Serial.println(" bytes");  
}

//=====================================================================
//=====================================================================
void updateOutput()
{
  //set failsafe flag
  bool failsafe = hxrcSlave.getReceiverStats().isFailsafe();
  hxMavlinkRCEncoder.setFailsafe( failsafe);
  
  //inject RSSI into channel 16
  hxMavlinkRCEncoder.setChannelValue( USE_MAVLINK_V1 ? MAVLINK_RC_CHANNELS_COUNT_V1 - 1 : MAVLINK_RC_CHANNELS_COUNT-1, 1000 + ((uint16_t)hxrcSlave.getReceiverStats().getRSSI())*10 );

  if ( !failsafe ) //keep last channel values on failsafe
  {
    HXRCChannels channels = hxrcSlave.getChannels();
    for ( int i = 0; i < MAVLINK_RC_CHANNELS_COUNT-1; i++)
    {
      hxMavlinkRCEncoder.setChannelValue( i, channels.getChannelValue(i) );
    }

    if ( state == RCV_NORMAL_MODE )
    {
      Serial.println("Rebooting to LR mode");
      delay(100);
      ESP.restart();
      delay(1000);
    }
    state = RCV_GOT_CONNECTION_ONCE;
  }

  hxMavlinkRCEncoder.loop( mavlinkSerial );
}

//=====================================================================
//=====================================================================
void loop()
{
  esp_task_wdt_reset();

  hxrcTelemetrySerial.flushIn();
  processIncomingTelemetry();
  
  fillOutgoingTelemetry();
  hxrcTelemetrySerial.flushOut();

  hxrcSlave.setA2(hxrcSlave.getReceiverStats().getRSSI());

  hxrcSlave.loop();
  
   if (millis() - lastStats > 1000)
  {
    lastStats = millis();

    Serial.print("Peer:");
    HXRCPrintMac(hxrcSlave.getPeerMac());
    
    Serial.print(" packets:");
    Serial.print(capture.packetsCount);

    Serial.print(" rssi:");
    Serial.print(capture.rssi);

    Serial.print(" noise_floor:");
    Serial.print(capture.noiseFloor);

    Serial.print(" rate:");
    Serial.println(capture.rate);

    hxrcSlave.getTransmitterStats().printStats();
    hxrcSlave.getReceiverStats().printStats();

    mavlinkFrameSender.dumpState();
 }

  updateOutput();

  if ( hxrcSlave.getReceiverStats().isFailsafe() )
  {
    ArduinoOTA.handle();  
  }

  if ( 
      USE_LR_MODE &&
      (state == RVC_WAITING_CONNECTION) && 
      ((NORMAL_MODE_DELAY_MS) > 0) &&
      ((millis() - startTime) > (NORMAL_MODE_DELAY_MS) ) 
      )
  {
    //if there is no transmitter connection after 1 minute after powerup, and LR more is enabled, 
    //switch to normal mode to show AP and allow OTA updates

    Serial.println("Switching to normal mode");

    state = RCV_NORMAL_MODE;

    if (esp_wifi_set_protocol (WIFI_IF_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N ) != ESP_OK)
    {
      Serial.println("HXRC: Error: Failed to enable normal mode");
    }

  }
}
 