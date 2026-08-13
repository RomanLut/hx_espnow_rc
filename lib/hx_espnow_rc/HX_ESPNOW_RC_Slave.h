#pragma once

#include <Arduino.h>

#include "HX_ESPNOW_RC_Base.h"

//=====================================================================
//=====================================================================
class HXRCSlave : public HXRCBase
{
private:

    volatile bool gotIncomingPacket;

    static HXRCSlave* pInstance;

#if defined(ESP32)
    portMUX_TYPE channelsMux;
#endif    
    HXRCChannels receivedChannels;

    HXRCSlavePayload outgoingData;
#if defined(ESP8266)
    static void OnDataSentStatic(uint8_t *mac_addr, uint8_t status);
    static void OnDataRecvStatic(uint8_t *mac, uint8_t *incomingData, uint8_t len);
    void OnDataSent(uint8_t *mac_addr, uint8_t status);
    void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len);
#elif defined (ESP32)
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0)
    static void OnDataSentStatic(const esp_now_send_info_t *sendInfo, esp_now_send_status_t status);
#else
    static void OnDataSentStatic(const uint8_t *mac_addr, esp_now_send_status_t status);
#endif
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    static void OnDataRecvStatic(const esp_now_recv_info_t *receiveInfo, const uint8_t *incomingData, int len);
#else
    static void OnDataRecvStatic(const uint8_t *mac, const uint8_t *incomingData, int len);
#endif
    void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
    void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);
#endif

public:

    HXRCSlave();
    virtual ~HXRCSlave();

    virtual bool init( HXRCConfig config ) override;
    virtual void loop() override;

    //index = 0..15
    //data = 1000...2000
    HXRCChannels getChannels();

    void setA1( uint32_t value);
    void setA2( uint32_t value);

};

