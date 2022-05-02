#pragma once

#include <Arduino.h> 
#include <ArduinoJson.h>

#define LED_PIN 2  //LED pin of ESP32. REVIEW: this def does not belong here

#define PROFILES_COUNT 11

/*
//=====================================================================
//=====================================================================
typedef enum 
{
  TM_CONFIG = 0,
  TM_ESPNOW = 1,            
  TM_BLUETOOTH_GAMEPAD = 2,
  TM_XIRO_MINI = 3,
  TM_BLE_GAMEPAD = 4,
  TM_E58 = 5,
  //TODO: other modes - Wifi RC, bluetooth RC, BLE RC 

} TransmitterMode;

//=====================================================================
//=====================================================================
typedef enum 
{
  SPE_RSSI = 0,         //output RSSI only
  SPE_PATHTHROUGH = 1,  //pass incoming telemetry to the SPORT pin
  //TODO: SPT_MAVLINK  mavlink to sport encoding

} SportTelemetryEncoder;
*/

//=====================================================================
//=====================================================================
class TXProfileManager
{
private:
  int currentProfileIndex;
  DynamicJsonDocument json;

  void loadConfig();
  void loadConfigProfile();
public:

  static TXProfileManager instance;  

/*
  TransmitterMode transmitterMode;

  SportTelemetryEncoder sportTelemetryEncoder;

  uint16_t espnow_key;
  uint8_t espnow_channel;
  bool espnow_lrMode;

  const char* ap_name;     //NULL - no AP
  const char* ap_password;  //NULL - no password

  const char* ftp_user;     //NULL - no ftp server
  const char* ftp_password;  //NULL - no password
*/
  TXProfileManager();

  JsonDocument* getCurrentProfile();
  int getCurrentProfileIndex();
  void setCurrentProfileIndex(int index);
};


/*

Actions

SetChannelOnEvent( channelNumber, eventId, eventParam, ValueType, value )

channelNumber:
- 0..14

EventId:
- STARTUP
- ALWAYS
- CHANNEL_LOWER_1500 eventParam:channelId
- CHANNEL_EQALS_1500 eventParam:channelId
- CHANNEL_GREATER_1500 eventParam:channelId

ValueType:
 CONSTANT value:value
 INPUT value:inputId
 TRIGGER
 SWITCH3
 SWITCH4
 INCREMENT value:increment
 DECREMENT value:decrement
 MUL value: mul

INPUT_ID:
  LEFT_STICK_X == AXIS1
  LEFT_STICK_X_INVERSE
  LEFT_STICK_Y
  LEFT_STICK_Y_INVERSE

  RIGHT_STICK_X 
  RIGHT_STICK_X_INVERSE
  RIGHT_STICK_Y
  RIGHT_STICK_Y_INVERSE

  AXIS_1  
  AXIS_1_INVERSE
  ...
  AXIS_N  
  AXIS_N_INVERSE

  LEFT_THUMB == BUTTON1

  RIGHT_THUMB == BUTTON2

  BUTTON1
  ...
  BUTTONN



*/