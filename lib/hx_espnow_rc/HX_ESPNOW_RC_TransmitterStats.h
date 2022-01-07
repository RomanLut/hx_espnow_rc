#pragma once

#include <Arduino.h>

#include "HX_ESPNOW_RC_Common.h"

//=====================================================================
//=====================================================================
class HXRCTransmitterStats
{
private:     

    void reset();
    
    void onPacketSendError();
    void onPacketSend( unsigned long timeMs );
    void onPacketSendMiss( uint16_t missedPackets );
    void onPacketAck( uint8_t telemetryLength );

    void update();

    friend class HXRCBase;
    friend class HXRCMaster;
    friend class HXRCSlave;

public:
    //number of packets send, including not sent due to API error
    uint16_t packetsSentTotal;
    uint16_t packetsAcknowledged;
    //API error
    uint16_t packetsSentError;
    //packets not sent in time because HXRCLoop() was not called in time
    uint16_t packetsNotSentInTime;  

    unsigned long lastSendTimeMs;
    unsigned long lastAcknowledgedPacketMs;

    uint16_t RSSIPacketsAcknowledged;
    uint16_t RSSIPacketsTotal;
    unsigned long RSSIUpdateMs;
    uint8_t RSSIlast;
    uint16_t successfullPacketRateLast; 

    uint32_t telemetryBytesSentTotal;
    uint32_t lastTelemetryBytesSentSpeed;
    uint32_t lastTelemetryBytesSentTotal;
    unsigned long telemetrySpeedUpdateMs;

    HXRCTransmitterStats();

    bool isFailsafe();
    uint8_t getRSSI();  //0..100 computed link quality
    uint8_t getSuccessfulPacketRate();  //successful packed per second

    // RSSIDbm, Noise floor, SNR and rate are available on ESP32 only. They are 0 on ESP8266 (rate is -1).
    uint8_t getRSSIDbm();  //harware RSSI in dbm. 70 means -70dbm
    uint8_t getNoiseFloor();  //noise floor in dbm. 90 means -90dbm
    uint8_t getSNR();  //signal to noise ratio in db
    int getRate();  //wifi rate, https://docs.espressif.com/projects/esp-idf/en/release-v3.3/api-reference/network/esp_wifi.html#_CPPv415wifi_phy_rate_t or -1 while not availabe

    void printStats();

    
    uint32_t getTelemetrySendSpeed();

};
