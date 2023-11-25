#pragma once

#include <Arduino.h>
#include "esp_camera.h"

//=========================================================
// LOW Quality Preset
#define DVR_LQ_FRAMESIZE        FRAMESIZE_HQVGA
#define DVR_LQ_FRAMESIZE_16x9   false
#define DVR_LQ_FPS              25
#define DVR_LQ_JPEG_QUALITY     12
#define DVR_LQ_SHARPNESS        -1
#define DVR_LQ_SATURATION       0

//=========================================================
// Medium Quality Preset
#define DVR_MQ_FRAMESIZE        FRAMESIZE_SVGA
#define DVR_MQ_FRAMESIZE_16x9   true
#define DVR_MQ_FPS              25
#define DVR_MQ_JPEG_QUALITY     5
#define DVR_MQ_SHARPNESS        0
#define DVR_MQ_SATURATION       2

//=========================================================
// High Quality Preset
#define DVR_HQ_FRAMESIZE        FRAMESIZE_HD
#define DVR_HQ_FRAMESIZE_16x9   true
#define DVR_HQ_FPS              25
#define DVR_HQ_JPEG_QUALITY     8
#define DVR_HQ_SHARPNESS        0
#define DVR_HQ_SATURATION       2

//=========================================================
// Generic settings

//0 to disable
//channel which controls quality
//<1250 - low
//1250...1750 - medium
//>1750 - high
#define QUALITY_CHANNEL 8  

//0 to disable
//channel which controls recording
//if specified, it will override onboard record button
//<=1750 - no recording
//>1750 - recording
#define RECORD_CHANNEL 14 

//true to autostart recording on boot
#define DVR_AUTO_START_RECORDING true

//max frames in one avi file (uint16_t)
//65000 25 FPS = 43 minutes
#define DVR_MAX_FRAMES  65000

//AVI size in seconds
#define DVR_MAX_TIME_SECONDS (5*60)

//should frames be sent if DVR is not recording?
#define DVR_SEND_FRAMES_NO_RECORDING false

//flip picture vetically
#define DVR_VERTICAL_FLIP 1

//mirror picture horizontally
#define DVR_HORIZONTAL_MIRROR 1

//=========================================================

//exclude HXRC functionality
//build DVR only without HXRC functionality (used for development)
//#define DVR_ONLY

//Exclude DVR functionality
//build HXRC only without DVR functionality (used for development)
//#define HXRC_ONLY

struct TDVRQualitySettings 
{
    framesize_t frameSize; //FRAMESIZE_SVGA etc
    bool frameSize16x9mod;  ////800x600 -> 800 x 452, implemented for 800x600 only
    int fps;  //25
    int jpegQuality; //0..63 lower is better
    int sharpness; //-2..2
    int saturation; //-2..2
};

extern int DVRQuality;
extern TDVRQualitySettings DVRQualitySettings[3];
