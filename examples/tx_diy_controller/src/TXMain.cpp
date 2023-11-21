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
void TXMain::dataFlowEvent()
{
    this->lastDataflowEvent = millis();
}

//=====================================================================
//=====================================================================
void TXMain::eventDataflowHandlerStatic()
{
  TXMain::instance.dataFlowEvent();
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
  if ( this->lastDataflowEvent + 300 > millis() )
  {
    if ( millis() & 32 )
    {
        v = 0;
    }
  }

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
HardwareSerial* TXMain::USBTelemetryOutputInitStatic(int baudRate)
{
  /*
    Serial0 - usb debug
    serial 1 - none
    serial 2 - bt
  */

  HXRCSetLogStream( externalBTSerial.getStream() );
  Serial.begin(baudRate);

  return &Serial;
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

  pinMode(HC06_INTERFACE_TX_PIN,OUTPUT);
  pinMode(HC06_INTERFACE_RX_PIN,INPUT);
  externalBTSerial.init(&Serial2, HC06_INTERFACE_RX_PIN, HC06_INTERFACE_TX_PIN);

  HXRCLOG.println("Start");

  digitalWrite(SPEAKER_PIN, LOW);
  pinMode(SPEAKER_PIN,OUTPUT);  //speaker pin

  ModeBase::USBTelemetryOutputInit = &USBTelemetryOutputInitStatic;
  ModeBase::eventDataFlowHandler = &TXMain::eventDataflowHandlerStatic;

  TXInput::instance.init();

  esp_task_wdt_reset();

  SPIFFS.begin(true); //true -> format if mount failed

  TXInput::instance.initAxisCalibrationData();
  TXInput::instance.loadAxisCalibrationData();

  this->loadLastProfile();

  ModeBase::currentModeHandler = &ModeIdle::instance;
  ModeBase::currentModeHandler->start(NULL, &externalBTSerial);

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

  //REVIEW: some library affects gpios?
  pinMode(JOY_LEFT_PIN,INPUT_PULLUP);
  pinMode(JOY_RIGHT_PIN,INPUT_PULLUP);

  TXInput::instance.loop(t);   //2ms sometimes

//uint32_t t1 = millis();

  StateBase::currentState->onRun(t);  //2-3ms

/*
t1 = millis() - t1;
if ( t1 > 1 )
{
  HXRCLOG.print("DT=");
  HXRCLOG.println(t1);
}
*/

  AudioManager::instance.loop(t); //10ms when sound is played

  this->bluetoothState.loop();  //0ms
  this->batteryState.loop();  //0ms


  //TXInput::instance.dumpBatADC();

  //digitalWrite(2, (t % 10) > 5 ? LOW : HIGH);
}


