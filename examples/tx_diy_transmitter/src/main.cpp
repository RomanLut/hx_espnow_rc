#include <Arduino.h>

#include <stdio.h>
#include <stdarg.h>

#include <esp_task_wdt.h>

#include "HX_ESPNOW_RC_Master.h"
#include "HX_ESPNOW_RC_SerialBuffer.h"

#include "txProfileManager.h"

#include "tx_config.h"
#include "HC06Interface.h"

#include "modeIdle.h"

#include "hx_channels.h"

static HC06Interface externalBTSerial;

#define ADC_COUNT 4
static uint32_t lastADCRead = millis();
static uint8_t ADC_PINS[ADC_COUNT]= {LEFT_STICK_X_PIN, LEFT_STICK_Y_PIN, RIGHT_STICK_X_PIN, RIGHT_STICK_Y_PIN};
static uint16_t ADC[ADC_COUNT];
static uint16_t ADCMin[ADC_COUNT];
static uint16_t ADCMax[ADC_COUNT];
static uint16_t ADCMidMin[ADC_COUNT];
static uint16_t ADCMidMax[ADC_COUNT];

static uint32_t lastButtonsRead = millis();
static uint8_t left_bumper_button;
static uint8_t right_bumper_button;

#define STATE_INIT                  0
#define STATE_RUN                   1
#define STATE_CALIBRATE_MIN_MAX     2
#define STATE_CENTER_WAIT           3
#define STATE_CALIBRATE_WAIT_CENTER 4
#define STATE_CALIBRATE_CENTER      5
static uint8_t state = STATE_INIT;

static uint32_t stateTime;


//=====================================================================
//=====================================================================
void initLedPin()
{
  pinMode(LED_PIN,OUTPUT);
  digitalWrite(LED_PIN, LOW );
}

//=====================================================================
//=====================================================================
void setLed( bool value )
{
  digitalWrite(LED_PIN, value ? HIGH : LOW );
}

//=====================================================================
//=====================================================================
void  setLEDS4(uint8_t v)
{
  digitalWrite(LED1_PIN, (v & 8)>0? HIGH:LOW );
  digitalWrite(LED2_PIN, (v & 4)>0? HIGH:LOW );
  digitalWrite(LED3_PIN, (v & 2)>0? HIGH:LOW );
  digitalWrite(LED4_PIN, (v & 1)>0? HIGH:LOW );
}


//=====================================================================
//=====================================================================
void onOTAprogress( uint a, uint b )  
{
  esp_task_wdt_reset();
}

//=====================================================================
//=====================================================================
void setup()
{
  esp_task_wdt_init(WDT_TIMEOUT_SECONDS, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch

  TXProfileManager::loadConfig();

  Serial.begin(115200, SERIAL_8N1);  

  HXRCLOG.println("Start");

  initLedPin();
  setLed(true);

  pinMode(13,OUTPUT);
  digitalWrite(13, HIGH );

  pinMode(LEFT_BUMPER_BTN,INPUT_PULLUP);
  pinMode(RIGHT_BUMPER_BTN,INPUT_PULLUP);

  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(LED4_PIN, OUTPUT);

  setLEDS4(0);

  externalBTSerial.init(&Serial2, HC06_INTERFACE_RX_PIN, HC06_INTERFACE_TX_PIN);

  ModeBase::currentModeHandler = &ModeIdle::instance;
  ModeBase::currentModeHandler->start();

  esp_task_wdt_reset();

  for ( int i = 0; i < ADC_COUNT; i++ )
  {
    ADC[i] = 2048;
    ADCMin[i] = 0;
    ADCMax[i] = 4096;
    ADCMidMin[i] = 2048-200;
    ADCMidMax[i] = 2048+200;
  }

  left_bumper_button = 0;
  right_bumper_button = 0;

  stateTime = millis();
}

//=====================================================================
//=====================================================================
void  getChannelValues( HXChannels* channelValues )
{
  channelValues-> isFailsafe = false;
  for ( int i = 0; i < HXRC_CHANNELS_COUNT; i++)
  {
    channelValues->channelValue[i] = 1000;
  }

  for ( int i = 0; i < ADC_COUNT; i++ )
  {
    channelValues->channelValue[i] = 1500;
    int v = ADC[i]>>2;
    if ( v < ADCMidMin[i] )
    {
        channelValues->channelValue[i] = map( v, ADCMidMin[i], ADCMin[i], 1500, 1000);
    }
    else if ( v > ADCMidMax[i] )
    {
        channelValues->channelValue[i] = map( v, ADCMidMax[i], ADCMax[i], 1500, 2000);
    }
  }

  channelValues->channelValue[ADC_COUNT+1] = ( left_bumper_button == 0 ) ? 2000: 1000;
  channelValues->channelValue[ADC_COUNT+2] = ( right_bumper_button == 0 ) ? 2000: 1000;

  channelValues->channelValue[15] = 1300;  //temp: BLE gamepad
}


//=====================================================================
//=====================================================================
//*v is current value * 4
void readADCEx(int pin, uint16_t* v )
{
  uint16_t sample[8];
  uint16_t mean = 0; 
  for ( int i = 0; i < 8; i++ )
  {
    sample[i]=analogRead(pin);
    mean += sample[i];
  }
  mean >>= 3;

  int32_t dev = 0;
  for ( int i = 0; i < 8; i++ )
  {
    int32_t dev0 = sample[i] - mean;
    dev += dev0*dev0;
  }

  dev = sqrt( dev );

  int count = 0;
  int mean1 = 0;
  for ( int i = 0; i < 8; i++ )
  {
    int16_t dev0 = sample[i] - mean;
    if ( dev0 < 0 ) dev0 = -dev0;
    if ( dev0 <= dev)
    {
      mean1+=sample[i];
      count++;
    }
  }

  if ( count > 0 )
  {
    mean = mean1 / count;
  }

  uint16_t v0 = ((*v)+2) >> 2;
  *v += -v0 + mean;
}

//=====================================================================
//=====================================================================
void  readADC(uint32_t t)
{
  if ( lastADCRead - t < 10 ) return;
  lastADCRead = t;

  for ( int i = 0; i < ADC_COUNT; i++ )
  {
    readADCEx(ADC_PINS[i], &ADC[i] );
  }
}

//=====================================================================
//=====================================================================
void  readButtons(uint32_t t)
{
  if ( lastButtonsRead - t < 10 ) return;
  lastButtonsRead = t;

  left_bumper_button <<= 1;
  if ( digitalRead(LEFT_BUMPER_BTN) == HIGH)
  {
    left_bumper_button |= 1;
  }

  right_bumper_button <<= 1;
  if ( digitalRead(RIGHT_BUMPER_BTN) == HIGH)
  {
    right_bumper_button |= 1;
  }
}

//=====================================================================
//=====================================================================
void startCalibrateMinMax()
{
  stateTime = millis();
  state = STATE_CALIBRATE_MIN_MAX;

  for ( int i = 0; i < ADC_COUNT; i++ )
  {
    ADCMin[i]=ADCMax[i]= 2048;
  }
}

//=====================================================================
//=====================================================================
void stateInitRun()
{
  int phase = (millis() - stateTime) / 500;
  switch (phase)
  {
    case 0: 
      setLEDS4( 8 ); 
      break;
    case 1: 
      setLEDS4( 8 + 4); 
      break;
    case 2: 
      setLEDS4( 8 + 4 + 2); 
      break;
    case 3: 
      setLEDS4( 8 + 4 + 2 + 1 ); 
      break;
     case 4: 

      if ( analogRead(LEFT_STICK_X_PIN) < 500 &&  analogRead(LEFT_STICK_Y_PIN) < 500 )
      {
        startCalibrateMinMax();
      }
      else
      {
        state = STATE_RUN;
      }
      break;
  }
}

//=====================================================================
//=====================================================================
void startCalibrateWaitCenter()
{
  stateTime = millis();
  state = STATE_CALIBRATE_WAIT_CENTER;

  for ( int i = 0; i < ADC_COUNT; i++ )
  {
    ADCMidMin[i]= ADCMidMax[i] = ADC[i];
  }
}

//=====================================================================
//=====================================================================
void startCalibrateCenter()
{
  stateTime = millis();
  state = STATE_CALIBRATE_CENTER;

  for ( int i = 0; i < ADC_COUNT; i++ )
  {
    ADCMidMin[i]= ADCMidMax[i] = ADC[i];
  }
}

//=====================================================================
//=====================================================================
void stateCalibrateMinMaxRun(uint32_t t )
{
  if ( (t % 1000) < 500 )
  {
      setLEDS4( 4 + 2 ); 
  }
  else
  {
      setLEDS4( 8 + 1 ); 
  }

  readADC(t);

  for ( int i = 0; i < ADC_COUNT; i++)
  {
    int v = ADC[i];
    if ( ADCMin[i] > v ) ADCMin[i] = v;
    if ( ADCMax[i] < v ) ADCMax[i] = v;
  }

  if ( t - stateTime > 7000)
  {
    stateTime = t;
    for ( int i = 0; i < ADC_COUNT; i++)
    {
      if ( ADCMin[i]> 500 || ADCMax[i]< (4096-500 ))
      {
        return;          
      }
    }
    for ( int i = 0; i < ADC_COUNT; i++)
    {
      ADCMin[i] >>= 2;
      ADCMax[i] >>= 2;
    }

    Serial.println("Min, Max:");
    Serial.printf( "%04d %04d %04d %04d\n", ADCMin[0], ADCMin[1], ADCMin[2], ADCMin[3] );
    Serial.printf( "%04d %04d %04d %04d\n", ADCMax[0], ADCMax[1], ADCMax[2], ADCMax[3] );

    startCalibrateWaitCenter();
  }


}

//=====================================================================
//=====================================================================
void stateCalibrateCenterWaitRun(uint32_t t )
{
  if ( (t % 1000) < 500 )
  {
      setLEDS4( 4 + 2 ); 
  }
  else
  {
      setLEDS4( 0 ); 
  }

  for ( int i = 0; i < ADC_COUNT; i++)
  {
    int v = ADC[i];
    if ( ADCMidMin[i] > v ) ADCMidMin[i] = v;
    if ( ADCMidMax[i] < v ) ADCMidMax[i] = v;
  }


  if ( t - stateTime > 1000)
  {
    stateTime = t;
    for ( int i = 0; i < ADC_COUNT; i++)
    {
      if ( (ADCMidMax[i] - ADCMidMin[i]) > 500 )
      {
        startCalibrateWaitCenter();
        return;          
      }
    }
    startCalibrateCenter();
  }
}

//=====================================================================
//=====================================================================
void stateCalibrateCenterRun(uint32_t t )
{
  setLEDS4( 4 + 2 ); 

  for ( int i = 0; i < ADC_COUNT; i++)
  {
    int v = ADC[i];
    if ( ADCMidMin[i] > v ) ADCMidMin[i] = v;
    if ( ADCMidMax[i] < v ) ADCMidMax[i] = v;
  }


  if ( t - stateTime > 1000)
  {
    stateTime = t;
    for ( int i = 0; i < ADC_COUNT; i++)
    {
      if ( (ADCMidMax[i] - ADCMidMin[i]) > 500 )
      {
        startCalibrateWaitCenter();
        return;          
      }
    }

    Serial.println("Middle Min, Max:");
    Serial.printf( "%04d %04d %04d %04d\n", ADCMidMin[0]>>2, ADCMidMin[1]>>2, ADCMidMin[2]>>2, ADCMidMin[3]>>2 );
    Serial.printf( "%04d %04d %04d %04d\n", ADCMidMax[0]>>2, ADCMidMax[1]>>2, ADCMidMax[2]>>2, ADCMidMax[3]>>2 );

    for ( int i = 0; i < ADC_COUNT; i++)
    {
      int d = ADCMidMax[i] - ADCMidMin[i];
      ADCMidMin[i] -= d/10 + 4;
      ADCMidMax[i] += d/10 + 4;

      ADCMidMin[i] >>=2;
      ADCMidMax[i] >>=2;
    }

    Serial.println("Middle Min, Max, delta, adjusted:");
    Serial.printf( "%04d %04d %04d %04d\n", ADCMidMin[0], ADCMidMin[1], ADCMidMin[2], ADCMidMin[3] );
    Serial.printf( "%04d %04d %04d %04d\n", ADCMidMax[0], ADCMidMax[1], ADCMidMax[2], ADCMidMax[3] );
    Serial.printf( "%04d %04d %04d %04d\n", (ADCMidMax[0] - ADCMidMin[0]), 
    (ADCMidMax[1] - ADCMidMin[1]), 
    (ADCMidMax[2] - ADCMidMin[2]), 
    (ADCMidMax[3] - ADCMidMin[3]));
    

    state = STATE_RUN;
  }

}

//=====================================================================
//=====================================================================
void stateRunRun()
{
    HXChannels channelValues;
    getChannelValues( &channelValues );

//    Serial.printf( "%04d %04d %04d %04d %03d %03d\n", ADC[0]>>2, ADC[1]>>2, ADC[2]>>2, ADC[3]>>2, left_bumper_button &0xf, right_bumper_button &0xf );

    ModeBase::currentModeHandler->loop( &channelValues, & externalBTSerial, NULL );

    setLEDS4( 8 ); 
}


//=====================================================================
//=====================================================================
void loop()
{
  esp_task_wdt_reset();

  uint32_t t = millis();

  readADC(t);
  readButtons(t);

  switch ( state )
  {
    case STATE_INIT:
      stateInitRun();
      break;

    case STATE_CALIBRATE_MIN_MAX:
      stateCalibrateMinMaxRun(t);
      break;

    case STATE_CALIBRATE_WAIT_CENTER:
      stateCalibrateCenterWaitRun(t);
      break;

    case STATE_CALIBRATE_CENTER:
      stateCalibrateCenterRun(t);
      break;

    case STATE_RUN:
      stateRunRun();
      break;
  }
}


