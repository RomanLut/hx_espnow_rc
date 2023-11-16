 #pragma once

#include <Arduino.h>

#define AVI_HEADER_LEN 240 // AVI header length
#define CHUNK_HDR 8 // bytes per jpeg hdr in AVI 

extern const uint8_t dcBuf[]; // 00dc

extern uint8_t aviHeader[AVI_HEADER_LEN];

extern void prepAviIndex(bool isTL = false);
extern void finalizeAviIndex(uint16_t frameCnt, bool isTL = false);
extern size_t writeAviIndex(byte* clientBuf, size_t buffSize, bool isTL = false);
extern void buildAviHdr(uint8_t FPS, uint8_t frameType, uint16_t frameCnt, bool isTL = false);
void buildAviIdx(size_t dataSize, bool isVid = true, bool isTL = false);




