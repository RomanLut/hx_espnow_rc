#pragma once

/*
            [][][][][][][][]
            [5V]      [3.3V]
            [GND]       [16]
 Mavlink RX [12]         [0]         <---/ ---> GND  FLASH Button
 Mavlink TX [13]       [GND]
            [15]     [3.3/5]
            [14]         [3] RX      <---/ ---> GND  REC Button
            [2]          [1] TX
       LAMP [4]      [GND/R]  <-- GND on some boards, RESET/EN (pullup 3.3V) on toher boards
            [  ESP32 CAM   ]
            [][][][][][][][]

Pinout:
https://randomnerdtutorials.com/esp32-cam-ai-thinker-pinout/

OV2640 Datasheet
https://github.com/Freenove/Freenove_ESP32_WROVER_Board/blob/main/Datasheet/OV2640/OV2640_DS(1.6).pdf
OV2640 Software application notes:
https://github.com/Freenove/Freenove_ESP32_WROVER_Board/blob/main/Datasheet/OV2640/OV2640%20Software%20Application%20Notes%201.03.pdf

V3660 Datasheet:
https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/docs/datasheet/unit/OV3660_CSP3_DS_1.3_sida.pdf
OV3640 Software application notes:
https://www.arducam.com/downloads/modules/OV3640/3-OV3640%20Software%20application%20notes.pdf

OV5640 Datahsheet:
https://cdn.sparkfun.com/datasheets/Sensors/LightImaging/OV5640_datasheet.pdf
OV5640 Software application notes:
https://www.waveshare.com/w/upload/c/cb/OV5640_camera_module_software_application_notes_1.3_Sonix.pdf
Hardware application notes:
http://ica123.com/wp-content/uploads/2021/01/OV5640_Camera_Module_Hardware_Application_Notes_R1.0%E8%8B%B1%E6%96%87%E6%89%8B%E5%86%8C.pdf
OV5640 Auto Focus Camera Module Application Note:
https://www.arducam.com/downloads/modules/OV5640/OV5640_Software_app_note_parallel.pdf
Timer tree:
https://patchwork.kernel.org/project/linux-media/patch/20181113130325.28975-2-maxime.ripard@bootlin.com/

Camera library:
https://github.com/espressif/esp32-camera

ov2640 image quality Issues:
https://github.com/espressif/esp32-camera/issues/203
https://github.com/raduprv/esp32-cam_ov2640-timelapse/blob/main/ov2640_timelapse_github.ino

Camera Settings explained:
https://github.com/mashabow/timelapse-esp32-cam/issues/4

Broken frames problem:
https://github.com/espressif/esp32-camera/issues/244#issuecomment-831561336

Sharpness, jpg clock:
https://github.com/espressif/esp32-camera/issues/203

Camera parallel interface description
https://www.ftdichip.com/old2020/Support/Documents/TechnicalNotes/TN_158_What_Is_The_Camera_Parallel_Interface.pdf

4 - LAMP
/33 - red LED
2,14,15 - SD 1 bit mode 4,12,13 - SD 4 bit mode
3 - RXD0 (debug)  
1 - TXD0 (debug)
12 - RXD2 (mavlink)
13 - TDX2 (mavlink)

Various notes:
1. Camera should be initialzied with highgest settings (UXGA, jpeg quality < 4) to preallocate large buffers. 
 Settings can be changed later. Otherwise app will likely crash on some high frame size.

2. Sharpness and denoise are not supported in esp32-camera library for ov2640.
 It is impossible to get good image outdoors with automatic sharpness. Camera enables blur when sky is seen in the large part of frame.
 So all you get is blurry footage in the air. Sharpness should be set to level 2 or 3 using direct registers manipulation.

3. SD Write speed in 1-bit mote is almost the same as in 4-bit mode. Should use 1-bit mode to free spare pins.

4. SD Write speed fluctuate a lot (updating FAT?)

5. ov2540 is capable of dumping 800x600 30FPS max on 24Mhz. esp32 can do 20Mhz only, so max fps is 30*20/24 = 25.

6. There are 3 'pixel-skipping' modes: 1600x1200, 800x600 and 640x480.  Other modes are just window inside theese modes.
So while 800x600 can do 25FPS, 1024x768 are significantly slover ( 12.5 FPS) because camera parses full 1600x1200 matrix.
It is not possible to get good FPS for 1280x720 unfortunatelly (should 3MP or 5MP camera improve situation?).

6. Windowing from 800x600 to 800x452 allows to decrease JPEG size by 30%.

7. Lamp LED is ~100mA. Better should be replaced with small LED + 300Ohm resistor.

8. Total power consumption without lamp is ~300mA.

9. Max SDCard size is 32GB.

*/

//true to use Mavlink1 ( TODO: support Mavlink1)
#define USE_MAVLINK_V1 false

//telemetry/mavlink port speed
#define TELEMETRY_BAUDRATE 115200

//send RC packet every ?ms
//Note for inav: "Serial receiver half-duplex" should be set to "OFF", otherwise inav will not be able to send telemetry.
#define MAVLINK_RC_PACKET_RATE_MS   40  //Rate in Hz = 1000 / MAVLINK_RC_PACKET_RATE_MS

//note: 
//Mavlink v1: 18 bytes(message size) * 25(rate) * 10(bits) = 4500 baud
//Mavlink v2: 34 bytes(message size) * 25(rate) * 10(bits) = 8500 baud

//=============================================================================
//Receiver binding
#define USE_WIFI_CHANNEL 3
#define USE_KEY 0 

#define USE_LR_MODE false

//if there is no transmitter connection after powerup to the specified time,
//receiver will switch from LR to normal mode to show AP and allow OTA updates
//set to 0 to disable
#define NORMAL_MODE_DELAY_MS 60*1000