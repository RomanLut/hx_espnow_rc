#pragma once

#include <Arduino.h>

#include "HX_ESPNOW_RC_Common.h"
#include "HX_ESPNOW_RC_Config.h"
#include "HX_ESPNOW_RC_SlavePayload.h"
#include "HX_ESPNOW_RC_MasterPayload.h"
#include "HX_ESPNOW_RC_RingBuffer.h"
#include "HX_ESPNOW_RC_TransmitterStats.h"
#include "HX_ESPNOW_RC_ReceiverStats.h"

//=====================================================================
//=====================================================================
class HXRCBase
{
protected:

    HXRCConfig config;

    HXRCTransmitterStats transmitterStats;
    HXRCReceiverStats receiverStats;
    
    volatile HXRCSenderStateEnum senderState;

    HXRCRingBuffer<HXRC_TELEMETRY_BUFFER_SIZE> incomingTelemetryBuffer;
    HXRCRingBuffer<HXRC_TELEMETRY_BUFFER_SIZE> outgoingTelemetryBuffer;

    void updateLed();

public:

    HXRCBase();
    virtual ~HXRCBase();

    virtual bool init(HXRCConfig config);
    virtual void loop();

    //return portion of incoming telemetry into buffer pBuffer which has size maxSize
    //returns size of returned data
    uint16_t getIncomingTelemetry(uint16_t maxSize, uint8_t* pBuffer);

    //add size byter from *ptr buffer to outgoing telemetry stream
    //returns true if bytes where added sucessfully
    //return false if buffer is overflown
    //As packet sensing is done from loop thread, 
    //we can send at most HXRC_MASTER_TELEMETRY_SIZE_MAX bytes every loop.
    bool sendOutgoingTelemetry( uint8_t* ptr, uint16_t size );

    HXRCTransmitterStats& getTransmitterStats();
    HXRCReceiverStats& getReceiverStats();
};

