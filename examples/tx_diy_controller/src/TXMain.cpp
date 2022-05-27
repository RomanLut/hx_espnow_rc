#include "TXMain.h"
#include "TXInput.h"
#include "StateInit.h"
#include "AudioManager.h"

#define BLUETOOTH_CHECK_PERIOD_MS 3000

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
#ifdef LED_INVERT  
  digitalWrite(LED1_PIN, HIGH);
  digitalWrite(LED2_PIN, HIGH);
  digitalWrite(LED3_PIN, HIGH);
  digitalWrite(LED4_PIN, HIGH);
#endif
}

//=====================================================================
//=====================================================================
void TXMain::setLEDS4(uint8_t v)
{
 #ifdef LED_INVERT
  digitalWrite(LED1_PIN, (v & 8)>0? LOW:HIGH );
  digitalWrite(LED2_PIN, (v & 4)>0? LOW:HIGH );
  digitalWrite(LED3_PIN, (v & 2)>0? LOW:HIGH );
  digitalWrite(LED4_PIN, (v & 1)>0? LOW:HIGH );
 #else
  digitalWrite(LED1_PIN, (v & 8)>0? HIGH:LOW );
  digitalWrite(LED2_PIN, (v & 4)>0? HIGH:LOW );
  digitalWrite(LED3_PIN, (v & 2)>0? HIGH:LOW );
  digitalWrite(LED4_PIN, (v & 1)>0? HIGH:LOW );
#endif  
}

//=====================================================================
//=====================================================================
void TXMain::setup()
{
  this->initLedPin();
  this->setLed(true);

  this->initLEDS4Pins();
  this->setLEDS4(0);

  pinMode(HC06_LED_PIN, INPUT);

  esp_task_wdt_init(WDT_TIMEOUT_SECONDS, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch

  Serial.begin(115200, SERIAL_8N1);  

  externalBTSerial.init(&Serial2, HC06_INTERFACE_RX_PIN, HC06_INTERFACE_TX_PIN);

  HXRCLOG.println("Start");

  digitalWrite(SPEAKER_PIN, LOW);
  pinMode(SPEAKER_PIN,OUTPUT);  //speaker pin

  pinMode(HC06_INTERFACE_TX_PIN,OUTPUT);
  pinMode(HC06_INTERFACE_RX_PIN,INPUT);

  TXInput::instance.init();

  esp_task_wdt_reset();

  SPIFFS.begin(true); //true -> format if mount failed

  TXInput::instance.initAxisCalibrationData();
  TXInput::instance.loadAxisCalibrationData();

  this->loadLastProfile();

  ModeBase::currentModeHandler = &ModeIdle::instance;
  ModeBase::currentModeHandler->start(NULL);

  AudioManager::instance.init();

  StateBase::Goto(&StateInit::instance);

  this->bluetoothCheckTime = millis() + 5000;
  this->lastBluetoothState = false;
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
void TXMain::checkBluetoothState(uint32_t t)
{
  if ( t > (this->bluetoothCheckTime + BLUETOOTH_CHECK_PERIOD_MS ))
  {
    this->bluetoothCheckTime = t;
    bool b = digitalRead(HC06_LED_PIN) == HIGH;
    if ( this->lastBluetoothState != b )
    {
      this->lastBluetoothState = b;
      AudioManager::instance.play( b ? "/bt_connected.mp3" : "/bt_disconnected.mp3", AUDIO_GROUP_NONE );
    }
  }

  //Serial.println( digitalRead(HC06_LED_PIN) == HIGH ? "H" : "L");
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

  this->checkBluetoothState(t);

  //TXInput::instance.dumpBatADC();

  //digitalWrite(2, (t % 10) > 5 ? LOW : HIGH);
}


