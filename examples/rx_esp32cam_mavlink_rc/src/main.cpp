#include <Arduino.h>

#include "rx_config.h"
#include "dvr_config.h"

#define LED_PIN 33
#define REC_BUTTON_PIN 3

#include <hxfilelog.h>

#ifndef DVR_ONLY
#include <ArduinoOTA.h>

#include "HX_ESPNOW_RC_Slave.h"
#include "hx_mavlink_rc_encoder.h"
#include "hx_mavlink_parser.h"
#include "hx_mavlink_frame_sender.h"
#include "HX_ESPNOW_RC_SerialBuffer.h"

#include <esp_task_wdt.h>

#define WDT_TIMEOUT_SECONDS 5  

#define ERROR_PAUSE_MS 10000

HXRCSlave hxrcSlave;
HXRCSerialBuffer<1024> hxrcTelemetrySerial( &hxrcSlave );
HXMavlinkRCEncoder hxMavlinkRCEncoder;
HXMavlinkParser outgoingMavlinkParser(true);
HXMavlinkFrameSender mavlinkFrameSender;

unsigned long lastStats = millis();

#endif

#ifdef HXRC_ONLY
#include "frame.h"
#endif

bool initError = false;
bool cameraInitError = false;
unsigned long DVRPause;

#ifndef DVR_ONLY

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

#endif 

// start SD **************************************************************

#include <SD_MMC.h>

HXFileLog<2048> fileLog;

#define ONEMEG (1024 * 1024)

//=====================================================================
//=====================================================================
char* fmtSize (uint64_t sizeVal) 
{
  static char returnStr[20];
  if (sizeVal < 50 * 1024) sprintf(returnStr, "%llu bytes", sizeVal);
  else if (sizeVal < ONEMEG) sprintf(returnStr, "%lluKB", sizeVal / 1024);
  else if (sizeVal < ONEMEG * 1024) sprintf(returnStr, "%0.1fMB", (double)(sizeVal) / ONEMEG);
  else sprintf(returnStr, "%0.1fGB", (double)(sizeVal) / (ONEMEG * 1024));
  return returnStr;
}

//=====================================================================
//=====================================================================
void printSDInfo()
{
  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) 
  {
    Serial.println("No SD card attached");
    initError = true;
    DVRPause = millis() + ERROR_PAUSE_MS;
  }
  else 
  {
    char typeStr[8] = "UNKNOWN";
    if (cardType == CARD_MMC) strcpy(typeStr, "MMC");
    else if (cardType == CARD_SD) strcpy(typeStr, "SDSC");
    else if (cardType == CARD_SDHC) strcpy(typeStr, "SDHC");
    Serial.printf("SD card type %s, Size: %s\n", typeStr, fmtSize(SD_MMC.cardSize()));
  }
}

//=====================================================================
//=====================================================================
bool initSD() 
{
  /* open SD card in MMC 1 bit mode
     MMC4  MMC1  ESP32 ESP32S3
      D2          12
      D3    ..    13
      CMD  CMD    15    38
      CLK  CLK    14    39
      D0   D0     2     40
      D1          4
  */

  bool res = false;
  heap_caps_malloc_extmem_enable(5); // small number to force vector into psram
  //fileVec.reserve(1000);
  heap_caps_malloc_extmem_enable(4096);

  res = SD_MMC.begin("/sdcard", true, false);

  pinMode(4, OUTPUT);
  digitalWrite(4, LOW); // set lamp pin fully off as sd_mmc library still initialises pin 4 in 1 line mode

  if (res) 
  {
    printSDInfo();
  } 
  else 
  {
    Serial.println("SD card mount failed");
    initError = true;
    DVRPause = millis() + ERROR_PAUSE_MS;
  }
  return res;
    
}

// end SD **************************************************************


// begin camera **************************************************************

#define CAMERA_MODEL_AI_THINKER 

#include "esp_camera.h"
#include "camera_pins.h"

#define FB_BUFFERS 3 // min 2, use max as much as PSRAM allows, to compensate fluctuating SD write speed. 1 buffer = ~400kb 
//TODO: can buffer 3x more if use own caching

uint8_t xclkMhz = 20; // camera clock rate MHz

//reg,val, mask
const static uint8_t OV2640_SHARPNESS_AUTO[]=   
{   
    0x92,   0x01,   0xff,   
    0x93,   0x20,   0x20,   
};   
   
const static uint8_t OV2640_SHARPNESS_MANUAL[]=   
{   
    0x92,   0x01,   0xff,   
    0x93,   0x00,   0x20,   
};   
   
const static uint8_t OV2640_SHARPNESS_LEVEL0[]=   
{   
    0x92,   0x01,   0xff,   
    0x93,   0xc0,   0x1f,   
};   
const static uint8_t OV2640_SHARPNESS_LEVEL1[]=   
{   
    0x92,   0x01,   0xff,   
    0x93,   0xc1,   0x1f,   
};   
const static uint8_t OV2640_SHARPNESS_LEVEL2[]=   
{   
    0x92,   0x01,   0xff,   
    0x93,   0xc2,   0x1f,   
};   
const static uint8_t OV2640_SHARPNESS_LEVEL3[]=   
{   
    0x92,   0x01,   0xff,   
    0x93,   0xc4,   0x1f,   
};   
const static uint8_t OV2640_SHARPNESS_LEVEL4[]=   
{   
    0x92,   0x01,   0xff,   
    0x93,   0xc8,   0x1f,   
};   
const static uint8_t OV2640_SHARPNESS_LEVEL5[]=   
{   
    0x92,   0x01,   0xff,   
    0x93,   0xd0,   0x1f,   
};   
const static uint8_t OV2640_SHARPNESS_LEVEL6[]=   
{   
    0x92,   0x01,   0xff,   
    0x93,   0xdf,   0x1f,   
};   
   
const static uint8_t* OV_SETTING_SHARPNESS[]=   
{   
    OV2640_SHARPNESS_LEVEL0,   
    OV2640_SHARPNESS_LEVEL1,   
    OV2640_SHARPNESS_LEVEL2,   
    OV2640_SHARPNESS_LEVEL3,   
    OV2640_SHARPNESS_LEVEL4,   
    OV2640_SHARPNESS_LEVEL5,   
    OV2640_SHARPNESS_LEVEL6   
};   


//=====================================================================
//=====================================================================
//0...5
void setOV2640SharpnessLevel( sensor_t * s, int sharpness)
{
/*
    reg, mask, val
    s->set_reg(s,0xff,0xff,0x00);//banksel:DSP   BANK_DSP, BANK_SENSOR, BANK_MAX
    //no sharpening
    s->set_reg(s,0x92,0xff,0x1);
    s->set_reg(s,0x93,0xff,0x0);  
*/

  s->set_reg(s,0xff,0xff,0x00); //banksel:DSP   BANK_DSP, BANK_SENSOR, BANK_MAX

  s->set_reg(s,OV2640_SHARPNESS_MANUAL[0],OV2640_SHARPNESS_MANUAL[2],OV2640_SHARPNESS_MANUAL[1]);
  s->set_reg(s,OV2640_SHARPNESS_MANUAL[0+3],OV2640_SHARPNESS_MANUAL[2+3],OV2640_SHARPNESS_MANUAL[1+3]);

  s->set_reg(s,OV_SETTING_SHARPNESS[sharpness][0],OV_SETTING_SHARPNESS[sharpness][2],OV_SETTING_SHARPNESS[sharpness][1]);
  s->set_reg(s,OV_SETTING_SHARPNESS[sharpness][0+3],OV_SETTING_SHARPNESS[sharpness][2+3],OV_SETTING_SHARPNESS[sharpness][1+3]);
}

typedef struct {
        union {
                struct {
                        uint8_t pclk_div:7;
                        uint8_t pclk_auto:1;
                };
                uint8_t pclk;
        };
        union {
                struct {
                        uint8_t clk_div:6;
                        uint8_t reserved:1;
                        uint8_t clk_2x:1;
                };
                uint8_t clk;
        };
} ov2640_clk_t;

#define R_DVP_SP            0xD3
#define CLKRC               0x11


//=====================================================================
//=====================================================================
void setDRVQuality() 
{
  sensor_t * s = esp_camera_sensor_get();

  s->set_framesize(s, DVRQualitySettings[DVRQuality].frameSize);
  if ( DVRQualitySettings[DVRQuality].frameSize16x9mod )
  {
    if ( DVRQualitySettings[DVRQuality].frameSize == FRAMESIZE_SVGA)
    {
      //800x452
      //ov2640_sensor_mode_t, unused,unused,unused, ofsx, ofsy, max_x, max_y, w, h
      s->set_res_raw(s, 1/*OV2640_MODE_SVGA*/,0,0,0, 0, 72, 800, 600-144, 800,600-144,false,false);
    }
    else if ( DVRQualitySettings[DVRQuality].frameSize == FRAMESIZE_UXGA)
    {
      //1600x904
      //ov2640_sensor_mode_t, unused,unused,unused, ofsx, ofsy, max_x, max_y, w, h
      s->set_res_raw(s, 0/*OV2640_MODE_UXGA*/,0,0,0, 0, 148, 1600, 1200-148*2, 1600,1200-148*2,false,false);
    }
  }

  if ( s->id.PID == OV2640_PID )
  {
    setOV2640SharpnessLevel( s, DVRQualitySettings[DVRQuality].sharpness + 3 );
  }
  else
  {
    s->set_sharpness(s, DVRQualitySettings[DVRQuality].sharpness);  //not supported on OV2640 in esp32camp library
  }

  s->set_saturation(s, DVRQualitySettings[DVRQuality].saturation);
  
  s->set_quality(s, DVRQualitySettings[DVRQuality].jpegQuality);  

  if (s->id.PID == OV2640_PID)
  {
    if (DVRQualitySettings[DVRQuality].frameSize == FRAMESIZE_SVGA)
    {
      s->set_xclk( s, 0, 12 );  //actual frequency will be 12,307692307692  (160Mhz / 13)

      ov2640_clk_t c;
      c.reserved = 0;

      c.clk_2x = 1;  // 12,307692307692 * 2 = ~ 24,6Mhz
      c.clk_div = 0;
      c.pclk_auto = 0;
      c.pclk_div = 6;    //DVP PCLK = XCLK * (clk_2x?2:1) / clk_div * 2 / pclk_div ????

      s->set_reg(s,0xff,0xff,0x01); //banksel:sensor  ()  BANK_DSP, BANK_SENSOR, BANK_MAX)
      s->set_reg(s, CLKRC, 0xff, c.clk);

      s->set_reg(s,0xff,0xff,0x00); //banksel:dsp  ()  BANK_DSP, BANK_SENSOR, BANK_MAX)
      s->set_reg(s, R_DVP_SP, 0xff, c.pclk);
    }
    else
    {
      s->set_xclk( s, 0, 20 );
    }
  }

}

//=====================================================================
//=====================================================================
void initCamera() 
{
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = xclkMhz * 1000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_LATEST;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  
  //init with high specs to pre-allocate large buffers
  config.frame_size = FRAMESIZE_UXGA;  
  config.jpeg_quality = 1;  
  //https://github.com/espressif/esp32-camera/issues/185#issue-716800775
  //jpeg_quality 0 ..5 -- 960,000 bytes uxga, 240,000 bytes svga
  //jpeg_quality 6..10 -- 384,000 bytes uxga, 96,000 bytes svga
  //jpeg_quality 11+ -- 240,000 bytes uxga, 60,000 bytes svga  */
  config.fb_count = FB_BUFFERS + 1; // +1 needed

  esp_err_t err = ESP_FAIL;
  uint8_t retries = 2;
  while (retries && (err != ESP_OK)) 
  {
    err = esp_camera_init(&config);
    if (err != ESP_OK) 
    {
      // power cycle the camera, provided pin is connected
      digitalWrite(PWDN_GPIO_NUM, 1);
      delay(100);
      digitalWrite(PWDN_GPIO_NUM, 0); 
      delay(100);
      retries--;
    }
  } 
    
  if (err != ESP_OK) 
  {
    Serial.printf("Startup failure: Camera init error 0x%x", err);
    initError = true;
    cameraInitError = true;
    DVRPause = millis() + 0xf000000;
    return;
  }

  Serial.print("Camera model: ");

  sensor_t * s = esp_camera_sensor_get();
  switch (s->id.PID) 
  {
    case (OV2640_PID):
      Serial.print( "OV2640\n" );
    break;

    case (OV3660_PID):
      Serial.print( "OV3660\n" );
    break;

    case (OV5640_PID):
      Serial.print( "OV5640\n" );
    break;

    default:
      Serial.print( "Other\n" );
    break;
  }

  setDRVQuality();

  //https://randomnerdtutorials.com/esp32-cam-ov2640-camera-settings/
  s->set_contrast(s, 0); //-2...2
  s->set_brightness(s, 0); //-2...2 
  //s->set_denoise(s, 4);  not supported on OV2640 in library
  s->set_gainceiling(s, (gainceiling_t)3);
  s->set_colorbar(s, 0);  //0 or 1 - enable color bars
  s->set_whitebal(s, 1);  //0 or 1
  s->set_gain_ctrl(s, 1);
  s->set_exposure_ctrl(s, 1);
  s->set_hmirror(s, 0);
  s->set_vflip(s, 0);
  s->set_awb_gain(s, 1);
  s->set_agc_gain(s, 0);
  s->set_aec_value(s, 204);
  s->set_aec2(s, 1);
  s->set_dcw(s, 1);
  s->set_bpc(s, 1);
  s->set_wpc(s, 1);
  s->set_raw_gma(s, 1);
  s->set_lenc(s, 1);
  s->set_special_effect(s, 0);
  s->set_wb_mode(s, 0);
  s->set_ae_level(s, 1);  
  
  s->set_vflip(s, DVR_VERTICAL_FLIP);
  s->set_hmirror(s, DVR_HORIZONTAL_MIRROR);

}

// end camera **************************************************************



// begin dvr **************************************************************

#include "avi.h"

#define RAMSIZE (1024 * 16) // set this to multiple of SD card sector size (512 or 1024 bytes)

SemaphoreHandle_t aviMutex = NULL;
SemaphoreHandle_t frameSemaphore;
TaskHandle_t captureHandle = NULL;
//uint8_t FPS = DVR_FPS;
//static uint8_t saveFPS = DVR_FPS;
static uint16_t frameInterval; // units of 0.1ms between frames
bool isCapturing = false;
bool wasCapturing = false;
bool forceRecord = false; //Hold true to record
uint8_t minSeconds = 5; // default min video length (includes POST_MOTION_TIME)

// header and reporting info
static uint32_t vidSize; // total video size
static uint32_t maxFrameSize; 
static uint16_t frameCnt;
static uint32_t aviStartTime; // total overall time
static uint32_t dTimeTot; // total frame decode/monitor time
static uint32_t fTimeTot; // total frame buffering time
static uint32_t wTimeTot; // total SD write time
static uint32_t oTime; // file opening time
static uint32_t cTime; // file closing time
static uint32_t maxWriteTime; // maximum time to write block

static size_t highPoint;
static File aviFile;
uint8_t iSDbuffer[RAMSIZE + CHUNK_HDR];
#define FILE_NAME_LEN 64
static char aviFileName[FILE_NAME_LEN];

#define AVITEMP "/current.avi"

//=====================================================================
//=====================================================================
static void IRAM_ATTR frameISR() 
{
  // interrupt at current frame rate
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(captureHandle, &xHigherPriorityTaskWoken); // wake capture task to process frame
  if (xHigherPriorityTaskWoken == pdTRUE) portYIELD_FROM_ISR();
}

//=====================================================================
//=====================================================================
void controlFrameTimer(bool restartTimer, int FPS) 
{
  // frame timer control, timer 3 so dont conflict with cam
  static hw_timer_t* frameTimer = NULL;
  
  // stop current timer
  if (frameTimer) 
  {
    timerAlarmDisable(frameTimer);   
    timerDetachInterrupt(frameTimer); 
    timerEnd(frameTimer);
  }
  
  if (restartTimer) 
  {
    // (re)start timer 3 interrupt per required framerate
    frameTimer = timerBegin(3, 8000, true); // 0.1ms tick
    frameInterval = 10000 / FPS; // in units of 0.1ms 
    Serial.printf("Frame timer interval %ums for FPS %u\n", frameInterval/10, FPS); 
    timerAlarmWrite(frameTimer, frameInterval, true); 
    timerAttachInterrupt(frameTimer, &frameISR, true);
    timerAlarmEnable(frameTimer);
  }
}

//=====================================================================
//=====================================================================
static bool openAvi() 
{
  oTime = millis();

  // open avi file with temporary name 
  aviFile = SD_MMC.open(AVITEMP, FILE_WRITE);
  if ( !aviFile ) return false;
  oTime = millis() - oTime;
  Serial.printf("File opening time: %ums\n", oTime);

  aviStartTime = millis();
  frameCnt = fTimeTot = wTimeTot = dTimeTot = vidSize = maxFrameSize = maxWriteTime = 0;
  highPoint = AVI_HEADER_LEN; // allocate space for AVI header
  prepAviIndex();
  return true;
}

//=====================================================================
//=====================================================================
static bool closeAvi() 
{
  uint32_t vidDuration = millis() - aviStartTime;
  uint32_t vidDurationSecs = lround(vidDuration/1000.0);
  
  Serial.printf("\nCapture time %us, allowed min: %us\n", vidDurationSecs, minSeconds);

  cTime = millis();
  // write remaining frame content to SD
  aviFile.write(iSDbuffer, highPoint); 
  size_t readLen = 0;

  // save avi index
  finalizeAviIndex(frameCnt);
  do 
  {
    readLen = writeAviIndex(iSDbuffer, RAMSIZE);
    if (readLen) aviFile.write(iSDbuffer, readLen);
  } while (readLen > 0);

  // save avi header at start of file
  float actualFPS = (1000.0f * (float)frameCnt) / ((float)vidDuration);
  uint8_t actualFPSint = (uint8_t)(lround(actualFPS));  

  xSemaphoreTake(aviMutex, portMAX_DELAY); 
  buildAviHdr( actualFPSint, DVRQualitySettings[DVRQuality].frameSize, DVRQualitySettings[DVRQuality].frameSize16x9mod, frameCnt );
  xSemaphoreGive(aviMutex); 

  aviFile.seek(0, SeekSet); // start of file
  aviFile.write(aviHeader, AVI_HEADER_LEN); 
  aviFile.close();

  Serial.printf("Final SD storage time %lu ms\n", millis() - cTime);

  uint32_t hTime = millis();
  
  if (vidDurationSecs >= minSeconds) 
  {
    
    int n = 1;
    while (true)
    {
      sprintf(aviFileName, "/capture%02d.avi",n);
      if ( !SD_MMC.exists( aviFileName ) ) break;
      n++;
    }
    Serial.print("Saving: "); 
    Serial.println(aviFileName); 
    
    SD_MMC.rename(AVITEMP, aviFileName);
    Serial.printf("AVI close time %lu ms\n", millis() - hTime); 
    cTime = millis() - cTime;
    
    // AVI stats
    fileLog.printf("******** AVI recording stats ********\n");
    fileLog.printf("%s\n", aviFileName);
    fileLog.printf("Recorded %s\n", aviFileName);
    fileLog.printf("AVI duration: %u secs\n", vidDurationSecs);
    fileLog.printf("Number of frames: %u\n", frameCnt);
    fileLog.printf("Required FPS: %u\n", DVRQualitySettings[DVRQuality].fps);
    fileLog.printf("Actual FPS: %0.1f\n", actualFPS);
    fileLog.printf("File size: %s\n", fmtSize(vidSize));
    if (frameCnt) 
    {
      fileLog.printf("Average/max frame length: %u / %u bytes\n", vidSize / frameCnt, maxFrameSize);
      fileLog.printf("Average bandwidth: %u kb/s\n", (vidSize / (vidDurationSecs >0 ? vidDurationSecs : 1) ) / 1024);
      fileLog.printf("Average frame monitoring time: %u ms\n", dTimeTot / frameCnt);
      fileLog.printf("Average frame buffering time: %u ms\n", fTimeTot / frameCnt);
      fileLog.printf("Average/max frame storage time: %u / %u ms\n", wTimeTot / frameCnt, maxWriteTime);
    }   
    fileLog.printf("Average SD write speed: %u kB/s\n", ((vidSize / wTimeTot) * 1000) / 1024);
    fileLog.printf("File open / completion times: %u ms / %u ms\n", oTime, cTime);
    fileLog.printf("Busy: %u%%\n", std::min(100 * (wTimeTot + fTimeTot + dTimeTot + oTime + cTime) / vidDuration, (uint32_t)100));
    fileLog.printf("Free: heap %u, block: %u, pSRAM %u\n", ESP.getFreeHeap(), heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL), ESP.getFreePsram());
    fileLog.printf("*************************************\n");
    fileLog.flush();
    return true; 
  } 
  else 
  {
    // delete too small files if exist
    SD_MMC.remove(AVITEMP);
    Serial.printf("Insufficient capture duration: %u secs\n", vidDurationSecs); 
    return false;
  }
}

//=====================================================================
//=====================================================================
static bool saveFrame(camera_fb_t* fb) 
{
  // save frame on SD card
  uint32_t fTime = millis();
  // align end of jpeg on 4 byte boundary for AVI
  uint16_t filler = (4 - (fb->len & 0x00000003)) & 0x00000003; 
  size_t jpegSize = fb->len + filler;
  // add avi frame header
  memcpy(iSDbuffer+highPoint, dcBuf, 4); 
  memcpy(iSDbuffer+highPoint+4, &jpegSize, 4);
  highPoint += CHUNK_HDR;
  if (highPoint >= RAMSIZE) 
  {
    // marker overflows buffer
    highPoint -= RAMSIZE;
    if ( aviFile.write(iSDbuffer, RAMSIZE) != RAMSIZE) return false;

    // push overflow to buffer start
    memcpy(iSDbuffer, iSDbuffer+RAMSIZE, highPoint);
  }
  // add frame content
  size_t jpegRemain = jpegSize;
  uint32_t wTime = millis();
  while (jpegRemain >= RAMSIZE - highPoint) 
  {
    // write to SD when RAMSIZE is filled in buffer
    memcpy(iSDbuffer+highPoint, fb->buf + jpegSize - jpegRemain, RAMSIZE - highPoint);
    if ( aviFile.write(iSDbuffer, RAMSIZE) != RAMSIZE) return false;
    jpegRemain -= RAMSIZE - highPoint;
    highPoint = 0;
  } 
  wTime = millis() - wTime;
  wTimeTot += wTime;
  if ( maxWriteTime < wTime ) maxWriteTime = wTime;

  //Serial.printf("SD storage time %u ms\n", wTime);
  // whats left or small frame
  memcpy(iSDbuffer+highPoint, fb->buf + jpegSize - jpegRemain, jpegRemain);
  highPoint += jpegRemain;
  
  buildAviIdx(jpegSize); // save avi index for frame
  vidSize += jpegSize + CHUNK_HDR;
  if ( maxFrameSize < jpegSize ) maxFrameSize = jpegSize;
  frameCnt++; 
  fTime = millis() - fTime - wTime;
  fTimeTot += fTime;
//  Serial.printf("Frame processing time %u ms\n", fTime);
//  Serial.println("============================");
  return true;
}

//=====================================================================
//=====================================================================
void setDVRFPS() 
{
  controlFrameTimer(true, DVRQualitySettings[DVRQuality].fps);
}

//=====================================================================
//=====================================================================
static boolean processFrame() 
{
  // get camera frame
  bool res = true;
  bool restartAvi = false;
  uint32_t dTime = millis();

  if ( initError) return false;

  camera_fb_t* fb = esp_camera_fb_get();
  if (fb == NULL) 
  {
    return false;
  }

  if ( DVRPause < dTime )
  {
    if ( forceRecord || DVR_SEND_FRAMES_NO_RECORDING)
    {
      if (mavlinkFrameSender.isEmpty())
      {   
        mavlinkFrameSender.addFrame(fb->buf, fb->len);
      }
    }
  }

  int quality = DVRQuality;
  
  if ( DVRPause < dTime )
  {
    if ( (QUALITY_CHANNEL>0) && !hxrcSlave.getReceiverStats().isFailsafe())
    { 
        HXRCChannels channels = hxrcSlave.getChannels();
        int v = channels.getChannelValue(QUALITY_CHANNEL-1);
        if ( v < 1250 ) 
        {
          quality = 0;
        }
        else if ( v < 1750 ) 
        {
          quality = 1;
        }
        else
        {
          quality = 2;
        }
    }
  }

  if ( DVRPause < dTime )
  {
    if (forceRecord && !wasCapturing) 
    {
      Serial.println("Capture started.");
      if ( openAvi() )
      {
        wasCapturing = true;
      }
      else
      {
        Serial.println("ERROR: Unable to open avi file.");
        forceRecord = false;
        initError = true;
        DVRPause = millis() + ERROR_PAUSE_MS;
      }
    }
  }

  if (wasCapturing )  
  {
    Serial.print(".");

    // capture is ongoing
    dTimeTot += millis() - dTime;
    if ( !saveFrame(fb) )
    {
      Serial.print("ERROR: Error writing AVI file.");
      forceRecord = false;
      initError = true;
      DVRPause = millis() + ERROR_PAUSE_MS;
    }

    if (frameCnt >= DVR_MAX_FRAMES) 
    {
      Serial.printf("Auto closed recording after %u frames\n", DVR_MAX_FRAMES);
      restartAvi = true;
    }

    uint32_t vidDuration = millis() - aviStartTime;
    if (vidDuration >= (DVR_MAX_TIME_SECONDS * 1000)) 
    {
      Serial.printf("Auto closed recording after %us\n", DVR_MAX_TIME_SECONDS);
      restartAvi = true;
    }

    if ( DVRQuality != quality )
    {
      Serial.println("Auto closed recording to change quality");
      restartAvi = true;
    }
  }

  esp_camera_fb_return(fb);

  if ((!forceRecord || restartAvi) && wasCapturing ) 
  {
    wasCapturing = false;
    closeAvi();
  }

  if ( (wasCapturing == false) && ( DVRQuality != quality) )
  {
      DVRQuality = quality;
      Serial.printf("Setting quality to: %d", DVRQuality);
      setDRVQuality();
      setDVRFPS();
      DVRPause = millis() + 2000; //2 seconds pause to apply camera settings
  }

  return res;
}

//=====================================================================
//=====================================================================
static void captureTask(void* parameter) 
{
  // woken by frame timer when time to capture frame
  uint32_t ulNotifiedValue;
  while (true) 
  {
    ulNotifiedValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    if (ulNotifiedValue > FB_BUFFERS) 
    {
      ulNotifiedValue = FB_BUFFERS; // prevent too big queue if FPS excessive
    }
    // may be more than one isr outstanding if the task delayed by SD write or jpeg decode
    while (ulNotifiedValue-- > 0) processFrame();
  }
  vTaskDelete(NULL);
}

//=====================================================================
//=====================================================================
bool initDVR() 
{
  //Initialisation & prep for AVI capture
  //readSemaphore = xSemaphoreCreateBinary();
  //playbackSemaphore = xSemaphoreCreateBinary();

  Serial.println("Init DVR...");

  aviMutex = xSemaphoreCreateMutex();
  frameSemaphore = xSemaphoreCreateBinary();

  camera_fb_t* fb = esp_camera_fb_get();
  if (fb == NULL) 
  {
    initError = true;
    DVRPause = millis() + ERROR_PAUSE_MS;
    fileLog.printf("Init Error: Failed to get camera frame");
    fileLog.flush();
    return false;
  }
  else 
  {
    Serial.printf("Got frame: %dx%d %d bytes\n", fb->width,fb->height, fb->len);
    esp_camera_fb_return(fb);
  }
/*
  openAvi();
  for ( int i =0; i < 200; i++ )
  {
    camera_fb_t* fb = esp_camera_fb_get();
    saveFrame(fb);
    esp_camera_fb_return(fb);
    Serial.print(".");
  }
    Serial.println("*");
  closeAvi();
*/

  xTaskCreate(&captureTask, "captureTask", 1024 * 4, NULL, 5, &captureHandle);

  setDVRFPS();

  Serial.println("Init DVR done.");

  return true;
}

//=====================================================================
//=====================================================================
static void deleteTask(TaskHandle_t thisTaskHandle) 
{
  if (thisTaskHandle != NULL) vTaskDelete(thisTaskHandle);
  thisTaskHandle = NULL;
}

//=====================================================================
//=====================================================================
void endTasks() 
{
  deleteTask(captureHandle);
}

//=====================================================================
//=====================================================================
void OTAprereq() 
{
  forceRecord = false;
  DVRPause = millis() + 0xff0000;
  controlFrameTimer(false, 25);
  endTasks();
  esp_camera_deinit();
  delay(100);
}

// end dvr **************************************************************

//=====================================================================
//=====================================================================
void updateLEDs()
{
  if ( cameraInitError )
  {
    bool b = (millis() & 0x7f) > 0x40;
    b &= (millis() & 0x7ff) > 0x400;
    digitalWrite( LED_PIN, b ? LOW : HIGH);
    digitalWrite( 4, b ? HIGH : LOW);
    return;
  }

  if ( initError )
  {
    bool b = (millis() & 0x7f) > 0x40;
    digitalWrite( LED_PIN, b ? LOW : HIGH);
    digitalWrite( 4, b ? HIGH : LOW);
    return;
  }

  if (forceRecord)
  {
    bool b = (millis() & 0x7ff) > 0x400;
    digitalWrite( LED_PIN, b ? LOW : HIGH);
    digitalWrite( 4, b ? HIGH : LOW);
  }
  else
  {
    digitalWrite( LED_PIN, LOW);
    digitalWrite( 4, HIGH);
  }
}


//=====================================================================
//=====================================================================
void checkButton()
{
  static uint32_t debounceTime = millis() + 100;
  static bool lastButtonState = false;

  if ( debounceTime > millis() ) return;
  bool buttonState = digitalRead( REC_BUTTON_PIN ) == LOW;
  if ( buttonState != lastButtonState )
  {
    debounceTime = millis() + 100;
    lastButtonState = buttonState;

    Serial.println("Button pressed");

    if ( buttonState )
    {
      forceRecord = !forceRecord;
    }
  }
}

#ifndef DVR_ONLY

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
bool trySendFrame()
{
  hxrcTelemetrySerial.flushOut();

#ifdef HXRC_ONLY
  if (mavlinkFrameSender.isEmpty())
  {   
    mavlinkFrameSender.addFrame(frame, FRAME_DATA_SIZE);
  }
#endif  

  return mavlinkFrameSender.send( hxrcTelemetrySerial );
}

//=====================================================================
//=====================================================================
void fillOutgoingTelemetry()
{
  //keep free space (300b) in outgoing queue so any packet can be put in

  while ( (mavlinkSerial.available() > 0) && (hxrcTelemetrySerial.getAvailableForWrite() > 300) ) 
  {
    uint8_t c = mavlinkSerial.read();
//    hxrcTelemetrySerial.write(c);
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
    if ( !trySendFrame() )
    {
      break;
    }
  }
}

//=====================================================================
//=====================================================================
void onOTAprogress( uint a, uint b )  
{
  esp_task_wdt_reset();
}

#endif

//=====================================================================
//=====================================================================
void printFreeMem()
{
  size_t freeHeap = ESP.getFreeHeap();
  Serial.print("Free heap memory: ");
  Serial.print(freeHeap);
  Serial.println(" bytes");  

  size_t freePSRAM = ESP.getFreePsram();
  Serial.print("Free PSRAM: ");
  Serial.print(freePSRAM);
  Serial.println(" bytes");  
}

//=====================================================================
//=====================================================================
void setup()
{
#ifndef DVR_ONLY
  esp_task_wdt_init(WDT_TIMEOUT_SECONDS, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch
#endif 

  DVRPause = millis() + 1000;

  //Note: GPIO16 can not be used, it is used by PSRAM
  //GPIO33 is red /LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.begin(115200, SERIAL_8N1, 4, 1 );  //connect RX pin to LAMP(4) pin to free pin 3

#ifndef DVR_ONLY
  mavlinkSerial.begin( TELEMETRY_BAUDRATE, SERIAL_8N1, 12, 13); //rx tx

  hxMavlinkRCEncoder.init( MAVLINK_RC_PACKET_RATE_MS, USE_MAVLINK_V1 );

  HXRCConfig config (
          USE_WIFI_CHANNEL,
          USE_KEY,
          false, //LR mode
          -1, false);
  
  config.packetRatePeriodMS = HXRCConfig::PACKET_RATE_MAX;
  config.slaveTelemertyPayloadSize = HXRC_SLAVE_TELEMETRY_SIZE_MAX;
  config.wifiPhyRate =  WIFI_PHY_RATE_1M_L;

  hxrcSlave.init(config);

  //REVIEW: receiver does not work if AP is not initialized?
  WiFi.softAP("hxrccammavlink", NULL, USE_WIFI_CHANNEL);

  //corruped frames due to wifi
  //https://github.com/espressif/esp32-camera/issues/244
  esp_wifi_set_ps(WIFI_PS_NONE);


  ArduinoOTA.onProgress(&onOTAprogress);
  ArduinoOTA.begin();  
#endif

#ifndef HXRC_ONLY

  printFreeMem();

  initSD(); 

  //reinit mavlink serial pins after initSD()
// pinMode(12, INPUT);
//  pinMode(13, OUTPUT);

  initCamera();

  initDVR();

  pinMode(REC_BUTTON_PIN,  INPUT_PULLUP);

  forceRecord = DVR_AUTO_START_RECORDING; 
#endif  

  printFreeMem();
}

#ifndef DVR_ONLY
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
#endif 

//=====================================================================
//=====================================================================
void loop()
{
#ifndef DVR_ONLY
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

    printFreeMem();
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

#endif
  updateLEDs();

  if ( DVRPause < millis() )
  {
    if ((RECORD_CHANNEL > 0) && !hxrcSlave.getReceiverStats().isFailsafe())
    {
      HXRCChannels channels = hxrcSlave.getChannels();
      forceRecord = channels.getChannelValue(RECORD_CHANNEL-1) > 1750;
    }
    else
    {
      checkButton();
    }

    if ( initError && forceRecord )
    {
      Serial.println("Rebooting after errror.");
      delay(300);
      ESP.restart();
    }
  }
  
}
 