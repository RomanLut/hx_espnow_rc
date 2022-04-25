#pragma once

#define LED_PIN 2  //LED pin of ESP32. REVIEW: this def does not belong here

#define PROFILES_COUNT 11

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

//=====================================================================
//=====================================================================
class TXProfileManager
{
private:
  static int currentProfileIndex;
  static TXProfileManager profiles[PROFILES_COUNT];
public:

  TransmitterMode transmitterMode;

  SportTelemetryEncoder sportTelemetryEncoder;

  uint16_t espnow_key;
  uint8_t espnow_channel;
  bool espnow_lrMode;

  const char* ap_name;     //NULL - no AP
  const char* ap_password;  //NULL - no password

  const char* ftp_user;     //NULL - no ftp server
  const char* ftp_password;  //NULL - no password

  TXProfileManager();

  static const TXProfileManager* getCurrentProfile();
  static int getCurrentProfileIndex();
  static void setCurrentProfileIndex(int index);
  static void loadConfig();
};


