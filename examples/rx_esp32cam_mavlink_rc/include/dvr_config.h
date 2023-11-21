#pragma once

#include <Arduino.h>
#include "esp_camera.h"

//exclude HXRC functionality
//build DVR only without HXRC functionality (used for development)
//#define DVR_ONLY

//Exclude DVR functionality
//build HXRC only without DVR functionality (used for development)
//#define HXRC_ONLY

//=========================================================

//DVR frame size
#define DVR_FRAMESIZE  FRAMESIZE_SVGA

//clip frame to 16:9 aspect ratio?
//800x600 -> 800 x 480
#define DVR_FRAMESIZE_16x9 false

//DVR FPS
#define DVR_FPS 25

//true to autostart recording on boot
#define DVR_AUTO_START_RECORDING true

//max frames in one avi file (uint16_t)
//65000 35 FPS = 43 minutes
#define DVR_MAX_FRAMES  65000

//AVI size in seconds
#define DVR_MAX_TIME_SECONDS (5*60)

//should frames be sent if DVR is not recording?
#define DVR_SEND_FRAMES_NO_RECORDING false

//0...63 Lower number is higher quality
#define DVR_JPEG_QUALITY 4

//flip picture vetically
#define DVR_VERTICAL_FLIP 1

//mirror picture horizontally
#define DVR_HORIZONTAL_MIRROR 1
