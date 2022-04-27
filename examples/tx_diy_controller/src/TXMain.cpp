#include "TXMain.h"
#include "TXInput.h"
#include "StateInit.h"

uint16_t CH16 = 1300; //BLE gamepad profile
HC06Interface externalBTSerial;

//Audio audio(true, I2S_DAC_CHANNEL_LEFT_EN);

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

  //temp PIN 13 output 3.3V
  pinMode(13,OUTPUT);
  digitalWrite(13, HIGH );

  TXInput::instance.init();

  ModeBase::currentModeHandler = &ModeIdle::instance;
  ModeBase::currentModeHandler->start();

  esp_task_wdt_reset();

  SPIFFS.begin(true); //true -> format if mount failed

  TXProfileManager::loadConfig();
  TXInput::instance.initCalibrationData();
  TXInput::instance.loadCalibrationData();

  StateBase::Goto(&StateInit::instance);
}

//=====================================================================
//=====================================================================
void TXMain::loop()
{
  esp_task_wdt_reset();

  uint32_t t = millis();

  TXInput::instance.loop(t);
  StateBase::currentState->onRun(t);
}


