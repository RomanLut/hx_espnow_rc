#pragma once

#include <Arduino.h> 
#include <ArduinoJson.h>

#define LED_PIN 2  //LED pin of ESP32. REVIEW: this def does not belong here

#define CONFIG_PROFILE_INDEX 9

#define JSON_BUFFER_SIZE 8192

//=====================================================================
//=====================================================================
class TXProfileManager
{
private:
  int currentProfileIndex;
  DynamicJsonDocument json;

  void loadConfig( int profileIndex );
  void loadConfigProfile();
public:

  static TXProfileManager instance;  

  TXProfileManager();

  JsonDocument* getCurrentProfile();
  int getCurrentProfileIndex();
  void setCurrentProfileIndex(int index);
};
