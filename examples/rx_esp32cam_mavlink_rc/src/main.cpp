#include <Arduino.h>

#include "rx_config.h"
#include "dvr_config.h"

#define LED_PIN 33
#define REC_BUTTON_PIN 3

#ifndef DVR_ONLY
#include <ArduinoOTA.h>

#include "HX_ESPNOW_RC_Slave.h"
#include "hx_mavlink_rc_encoder.h"
#include "hx_mavlink_parser.h"
#include "hx_mavlink_frame_sender.h"
#include "HX_ESPNOW_RC_SerialBuffer.h"

#include <esp_task_wdt.h>

#define WDT_TIMEOUT_SECONDS 5  

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
  }
  return res;
    
}

// end SD **************************************************************


// begin camera **************************************************************

#define CAMERA_MODEL_AI_THINKER 

#include "esp_camera.h"
#include "camera_pins.h"

#define FB_BUFFERS 2 // stream / record

uint8_t xclkMhz = 20; // camera clock rate MHz

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
  // init with high specs to pre-allocate larger buffers
  config.fb_location = CAMERA_FB_IN_PSRAM;

  config.frame_size = DVR_FRAMESIZE;  

  config.jpeg_quality = DVR_JPEG_QUALITY;
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

  //https://randomnerdtutorials.com/esp32-cam-ov2640-camera-settings/

  s->set_framesize(s, DVR_FRAMESIZE);
  s->set_quality(s, DVR_JPEG_QUALITY);
  s->set_contrast(s, 0);
  s->set_brightness(s, 0);
  s->set_saturation(s, 0);
  s->set_denoise(s, 4);    
  s->set_sharpness(s, 0);    
  s->set_gainceiling(s, (gainceiling_t)0);
  s->set_colorbar(s, 0);
  s->set_whitebal(s, 1);
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
  s->set_ae_level(s, 2);  
  
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
uint8_t FPS = DVR_FPS;
static uint8_t saveFPS = DVR_FPS;
static uint16_t frameInterval; // units of 0.1ms between frames
bool isCapturing = false;
bool wasCapturing = false;
bool forceRecord = false; //Hold true to record
uint8_t minSeconds = 5; // default min video length (includes POST_MOTION_TIME)

// header and reporting info
static uint32_t vidSize; // total video size
static uint16_t frameCnt;
static uint32_t aviStartTime; // total overall time
static uint32_t dTimeTot; // total frame decode/monitor time
static uint32_t fTimeTot; // total frame buffering time
static uint32_t wTimeTot; // total SD write time
static uint32_t oTime; // file opening time
static uint32_t cTime; // file closing time

static size_t highPoint;
static File aviFile;
uint8_t iSDbuffer[(RAMSIZE + CHUNK_HDR) * 2];
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
void controlFrameTimer(bool restartTimer) 
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
static void openAvi() 
{
  oTime = millis();

  // open avi file with temporary name 
  aviFile = SD_MMC.open(AVITEMP, FILE_WRITE);
  oTime = millis() - oTime;
  Serial.printf("File opening time: %ums\n", oTime);

  aviStartTime = millis();
  frameCnt = fTimeTot = wTimeTot = dTimeTot = vidSize = 0;
  highPoint = AVI_HEADER_LEN; // allocate space for AVI header
  prepAviIndex();
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
  buildAviHdr(actualFPSint, DVR_FRAMESIZE, frameCnt);
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
    Serial.printf("******** AVI recording stats ********\n");
    Serial.printf("Recorded %s\n", aviFileName);
    Serial.printf("AVI duration: %u secs\n", vidDurationSecs);
    Serial.printf("Number of frames: %u\n", frameCnt);
    Serial.printf("Required FPS: %u\n", FPS);
    Serial.printf("Actual FPS: %0.1f\n", actualFPS);
    Serial.printf("File size: %s\n", fmtSize(vidSize));
    if (frameCnt) 
    {
      Serial.printf("Average frame length: %u bytes\n", vidSize / frameCnt);
      Serial.printf("Average frame monitoring time: %u ms\n", dTimeTot / frameCnt);
      Serial.printf("Average frame buffering time: %u ms\n", fTimeTot / frameCnt);
      Serial.printf("Average frame storage time: %u ms\n", wTimeTot / frameCnt);
    }   
    Serial.printf("Average SD write speed: %u kB/s\n", ((vidSize / wTimeTot) * 1000) / 1024);
    Serial.printf("File open / completion times: %u ms / %u ms\n", oTime, cTime);
    Serial.printf("Busy: %u%%\n", std::min(100 * (wTimeTot + fTimeTot + dTimeTot + oTime + cTime) / vidDuration, (uint32_t)100));
    Serial.printf("Free: heap %u, block: %u, pSRAM %u\n", ESP.getFreeHeap(), heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL), ESP.getFreePsram());
    Serial.printf("*************************************\n");
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
static void saveFrame(camera_fb_t* fb) 
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
    aviFile.write(iSDbuffer, RAMSIZE);
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
    aviFile.write(iSDbuffer, RAMSIZE);
    jpegRemain -= RAMSIZE - highPoint;
    highPoint = 0;
  } 
  wTime = millis() - wTime;
  wTimeTot += wTime;
  //Serial.printf("SD storage time %u ms\n", wTime);
  // whats left or small frame
  memcpy(iSDbuffer+highPoint, fb->buf + jpegSize - jpegRemain, jpegRemain);
  highPoint += jpegRemain;
  
  buildAviIdx(jpegSize); // save avi index for frame
  vidSize += jpegSize + CHUNK_HDR;
  frameCnt++; 
  fTime = millis() - fTime - wTime;
  fTimeTot += fTime;
//  Serial.printf("Frame processing time %u ms\n", fTime);
//  Serial.println("============================");
}

//=====================================================================
//=====================================================================
static boolean processFrame() 
{
  // get camera frame
  bool res = true;

  bool restartAvi = false;

  uint32_t dTime = millis();

  //Serial.println("PROCESS1");

  camera_fb_t* fb = esp_camera_fb_get();
  if (fb == NULL) 
  {
    return false;
  }
      //Serial.println("PROCESS2");

  if ( forceRecord || DVR_SEND_FRAMES_NO_RECORDING)
  {
    if (mavlinkFrameSender.isEmpty())
    {   
      mavlinkFrameSender.addFrame(fb->buf, fb->len);
    }
  }

  if (forceRecord && !wasCapturing) 
  {
    Serial.println("Capture started.");
    openAvi();
    wasCapturing = true;
  }

  if (wasCapturing )  
  {
    Serial.print(".");

    // capture is ongoing
    dTimeTot += millis() - dTime;
    saveFrame(fb);

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

  }

  esp_camera_fb_return(fb);

  if ((!forceRecord || restartAvi) && wasCapturing ) 
  {
    wasCapturing = false;
    closeAvi();
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
uint8_t setFPS(uint8_t val) 
{
  // change or retrieve FPS value
  if (val) 
  {
    FPS = val;
    // change frame timer which drives the task
    controlFrameTimer(true);
    saveFPS = FPS; // used to reset FPS after playback
  }
  return FPS;
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
    Serial.println("Error: Failed to get camera frame");
    initError = true;
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

  setFPS(DVR_FPS); 

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
  controlFrameTimer(false);
  endTasks();
  esp_camera_deinit();
  delay(100);
}

// end dvr **************************************************************

//=====================================================================
//=====================================================================
void updateLEDs()
{
  if ( initError )
  {
    digitalWrite( LED_PIN, (millis() & 0x7f) > 0x40 ? LOW : HIGH);
    digitalWrite( 4, (millis() & 0x7f) > 0x40 ? LOW : HIGH);
    return;
  }

  if (forceRecord)
  {
    digitalWrite( LED_PIN, (millis() & 0x7ff) > 0x400 ? LOW : HIGH);
    digitalWrite( 4, (millis() & 0x7ff) > 0x400 ? LOW : HIGH);
  }
  else
  {
    digitalWrite( LED_PIN, LOW);
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

  //Note: GPIO16 can not be used, it is used by PSRAM
  //GPIO33 is red /LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  //Serial.begin(115200, SERIAL_8N1, 12, 13); //debug output on GPIO13
  //mavlinkSerial.begin( TELEMETRY_BAUDRATE, SERIAL_8N1, 3, 1);  //Mavlink telemetry on GPIO3 and GPIO1 (use UART2 on default pins for UART0)

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
  checkButton();
}
 