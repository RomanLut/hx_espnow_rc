#pragma once

#define LED_PIN 2
#define USE_SERIAL1_RX_PIN 27
#define SPORT_PIN 18

#define CP2102_RX_PIN  1

//when SPORT support is enabled:
//- UART0 TX and RX pins are remaped from USB to SPORT_PIN
//- UART0 initialized to 57600 baud rate
//- Software UART 115200 is created and attached to USB
//- esp_log_set_vprintf() redirects ESP logging library to software uart
//- HXRCLOG is set to SoftwareSerial
#define USE_SPORT

//= Dependent definitions ========================================================
#ifdef USE_SPORT 
#else
#endif

#define PROFILES_COUNT 10

/*
//=====================================================================
//=====================================================================
void HXRCGetLogStream(const char *fmt, va_list args)
{
  char buffer[256];
  vsnprintf(buffer, 256, fmt, args);
  buffer[255] = 0;
  Serial.print ("Redirected:");
  Serial.print (buffer);

  return strlen(buffer);
}

*/

//=====================================================================
//=====================================================================
typedef enum 
{
  TM_ESPNOW = 0,            
  TM_BLUETOOTH_GAMEPAD = 1
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
class TXConfigProfile
{
private:
  static int currentProfileIndex;
  static TXConfigProfile profiles[PROFILES_COUNT];
public:

  TransmitterMode transmitterMode;

  SportTelemetryEncoder sportTelemetryEncoder;

  uint16_t espnow_key;
  uint8_t espnow_channel;
  bool espnow_lrMode;

  const char* ap_name;     //NULL - no AP
  const char* ap_password;  //NULL - no password

  TXConfigProfile();

  static const TXConfigProfile* getCurrentProfile();
  static int getCurrentProfileIndex();
  static void setCurrentProfileIndex(int index);
  static void loadConfig();
};


