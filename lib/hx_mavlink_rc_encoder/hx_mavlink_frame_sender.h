#pragma once

#include <Arduino.h>
#include <stdint.h>

#include "HX_ESPNOW_RC_SerialBuffer.h"

//https://mavlink.io/en/services/image_transmission.html

//=====================================================================
//=====================================================================
class HXMavlinkFrameSender
{
private:
    typedef enum
    {
        FS_IDLE         = 0,
        FS_FRAME_HEADER = 1,
        FS_CHUNK        = 3
    } RSState;

    uint8_t* frameData;
    int frameSize;

    uint8_t state;

    bool hasNewFrameData;
    int chunk;

    SemaphoreHandle_t frameDataMutex;

    bool trySendFrameHeader(HXRCSerialBufferBase& buffer);
    bool trySendChunk(HXRCSerialBufferBase& buffer);

public:
    HXMavlinkFrameSender();
    ~HXMavlinkFrameSender();

    bool send(HXRCSerialBufferBase& buffer);
    bool isEmpty();
    void addFrame(const uint8_t* frame, int size);
    void dumpState();
};



