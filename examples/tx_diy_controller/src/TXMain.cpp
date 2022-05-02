#include "TXMain.h"
#include "TXInput.h"
#include "StateInit.h"
#include "AudioManager.h"

uint8_t currentProfileIndex;
HC06Interface externalBTSerial;

TXMain TXMain::instance;

//=====================================================================
//=====================================================================
void TXMain::initLedPin()
{
  pinMode(LED_PIN,OUTPUT);
  digitalWrite(LED_PIN, LOW );
}

//=====================================================================
//=====================================================================
void TXMain::setLed( bool value )
{
  digitalWrite(LED_PIN, value ? HIGH : LOW );
}

//=====================================================================
//=====================================================================
void TXMain::initLEDS4Pins()
{
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(LED4_PIN, OUTPUT);
}

//=====================================================================
//=====================================================================
void TXMain::setLEDS4(uint8_t v)
{
  digitalWrite(LED1_PIN, (v & 8)>0? HIGH:LOW );
  digitalWrite(LED2_PIN, (v & 4)>0? HIGH:LOW );
  digitalWrite(LED3_PIN, (v & 2)>0? HIGH:LOW );
  digitalWrite(LED4_PIN, (v & 1)>0? HIGH:LOW );
}

//=====================================================================
//=====================================================================
void TXMain::setup()
{
  esp_task_wdt_init(WDT_TIMEOUT_SECONDS, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch

  Serial.begin(115200, SERIAL_8N1);  

  externalBTSerial.init(&Serial2, HC06_INTERFACE_RX_PIN, HC06_INTERFACE_TX_PIN);

  HXRCLOG.println("Start");

  this->initLedPin();
  this->setLed(true);

  this->initLEDS4Pins();
  this->setLEDS4(0);

  //temp PIN 14 output 3.3V
  pinMode(14,OUTPUT);
  digitalWrite(14, HIGH );

  pinMode(25,OUTPUT);  //speaker pin
  digitalWrite(25, LOW);

  TXInput::instance.init();

  esp_task_wdt_reset();

  SPIFFS.begin(true); //true -> format if mount failed

  TXInput::instance.initCalibrationData();
  TXInput::instance.loadCalibrationData();

  this->loadLastProfile();

  ModeBase::currentModeHandler = &ModeIdle::instance;
  ModeBase::currentModeHandler->start(NULL);

  AudioManager::instance.init();

  StateBase::Goto(&StateInit::instance);
}

//=====================================================================
//=====================================================================
void TXMain::loadLastProfile()
{
  currentProfileIndex = 3; //BLE_GAMEPAD

  File configFile = SPIFFS.open("/lastState.json");
  if  (!configFile) return;

  DynamicJsonDocument json(512);

  DeserializationError error = deserializeJson(json, configFile);
  configFile.close();

  if (error)
  {
    Serial.println(F("Failed to read file, using default configuration"));
    return;
  }

  currentProfileIndex = json["lastProfileIndex"] | 3;
}

//=====================================================================
//=====================================================================
void TXMain::saveLastProfile()
{
  File configFile = SPIFFS.open("/lastState.json", FILE_WRITE);
  if (!configFile) 
  {
    Serial.println("- failed to open config file for writing");
    return;
  }
  DynamicJsonDocument json(512);

  json["lastProfileIndex"] = currentProfileIndex;

  //serializeJsonPretty(json,Serial);                    
  serializeJsonPretty(json,configFile);                 
  configFile.close();
}


//=====================================================================
//=====================================================================
void TXMain::loop()
{
  esp_task_wdt_reset();

  uint32_t t = millis();

  TXInput::instance.loop(t);
  StateBase::currentState->onRun(t);

  AudioManager::instance.loop(t);

  //digitalWrite(25, (t % 10) > 5 ? LOW : HIGH);
}


