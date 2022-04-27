#include "TXInput.h"

const uint8_t TXInput::ADC_PINS[ADC_COUNT] = ADC_PINS_LIST
const uint8_t TXInput::BUTTON_PINS[BUTTONS_COUNT] = BUTTON_PINS_LIST

TXInput TXInput::instance;

//=====================================================================
//=====================================================================
TXInput::TXInput()
{
}

//=====================================================================
//=====================================================================
void TXInput::initAxisPins()
{
  for ( int i = 0; i < ADC_COUNT; i++ )
  {
    pinMode(ADC_PINS[i],INPUT);
  }
}

//=====================================================================
//=====================================================================
void TXInput::initButtonPins()
{
  for ( int i =0; i < BUTTONS_COUNT; i++)
  {
    pinMode(BUTTON_PINS[i],INPUT_PULLUP);
  }
}

//=====================================================================
//=====================================================================
void TXInput::init()
{
  this->lastADCRead = millis();
  this->lastButtonsRead = millis();

  this->initAxisPins();
  this->initButtonPins();
}


//=====================================================================
//=====================================================================
void TXInput::initCalibrationData()
{
  for ( int i = 0; i < ADC_COUNT; i++ )
  {
    this->ADC[i] = 2048;
    this->ADCMin[i] = 0;
    this->ADCMax[i] = 4096;
    this->ADCMidMin[i] = 2048-200;
    this->ADCMidMax[i] = 2048+200;
  }

  for ( int i = 0; i < BUTTONS_COUNT; i++)
  {
    this->buttonData[i] = 15;
  }
}

//=====================================================================
//=====================================================================
void TXInput::loadCalibrationData()
{
  File file = SPIFFS.open("/calibration.json");

  DynamicJsonDocument json(512);

  DeserializationError error = deserializeJson(json, file);
  if (error)
  {
    Serial.println(F("Failed to read file, using default configuration"));
    return;
  }

  for ( int i = 0; i < ADC_COUNT; i++ )
  {
    ADCMin[i] = json["axis"][i]["min"] | 0;
    ADCMax[i] = json["axis"][i]["max"] | 4096;
    ADCMidMin[i] = json["axis"][i]["midMin"] | (2048-200);
    ADCMidMax[i] = json["axis"][i]["midMax"] | (2048+200);
  }

  file.close();
}

//=====================================================================
//=====================================================================
void TXInput::saveCalibrationData()
{
  File configFile = SPIFFS.open("/calibration.json", FILE_WRITE);
  if (!configFile) 
  {
    Serial.println("- failed to open config file for writing");
    return;
  }
  DynamicJsonDocument json(512);

  for ( int i =0; i < ADC_COUNT; i++ )
  {
    json["axis"][i]["min"] = this->ADCMin[i];
    json["axis"][i]["max"] = this->ADCMax[i];
    json["axis"][i]["midMin"] = this->ADCMidMin[i];
    json["axis"][i]["midMax"] = this->ADCMidMax[i];
  }
  
  //serializeJsonPretty(json,Serial);                    
  serializeJsonPretty(json,configFile);                 
  configFile.close();
}

//=====================================================================
//=====================================================================
//*v is current value * 4
void TXInput::readADCEx(int pin, uint16_t* v )
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
void TXInput::readADC(uint32_t t)
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
void TXInput::readButtons(uint32_t t)
{
  if ( lastButtonsRead - t < 10 ) return;
  lastButtonsRead = t;

  for ( int i = 0; i < BUTTONS_COUNT; i++ )
  {
    buttonData[i] <<=1;
    if ( digitalRead(BUTTON_PINS[i]) == HIGH)
    {
      buttonData[i] |= 1;
    }
  }
}

//=====================================================================
//=====================================================================
void TXInput::getChannelValues( HXChannels* channelValues )
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

  for ( int i =0; i < BUTTONS_COUNT; i++ )
  {
    channelValues->channelValue[ADC_COUNT+i] = ( buttonData[i] == 0 ) ? 2000: 1000;
  }
}

//=====================================================================
//=====================================================================
bool TXInput::isCalibrateGesture()
{
  return analogRead(LEFT_STICK_X_PIN) < 500 &&  analogRead(LEFT_STICK_Y_PIN) < 500;
}

//=====================================================================
//=====================================================================
bool TXInput::isInitGesture()
{
  return analogRead(RIGHT_STICK_X_PIN) > 3500 &&  analogRead(RIGHT_STICK_Y_PIN) < 500;
}

//=====================================================================
//=====================================================================
void TXInput::calibrateInitADC()
{
  for ( int i = 0; i < ADC_COUNT; i++ )
  {
    ADCMin[i] = ADCMax[i] = 2048;
  }
}

//=====================================================================
//=====================================================================
void TXInput::calibrateAdjustMinMaxADC()
{
  for ( int i = 0; i < ADC_COUNT; i++)
  {
    int v = ADC[i];
    if ( ADCMin[i] > v ) ADCMin[i] = v;
    if ( ADCMax[i] < v ) ADCMax[i] = v;
  }
}

//=====================================================================
//=====================================================================
bool TXInput::isMinMaxCalibrationSuccessfull()
{
  for ( int i = 0; i < ADC_COUNT; i++)
  {
    if ( ADCMin[i] > 500 || ADCMax[i] < (4096-500 ))
    {
      return false;          
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

  return true;
}

//=====================================================================
//=====================================================================
void TXInput::calibrateInitADC2()
{
  for ( int i = 0; i < ADC_COUNT; i++ )
  {
    ADCMidMin[i]= ADCMidMax[i] = ADC[i];
  }
}

//=====================================================================
//=====================================================================
void TXInput::calibrateAdjustMidMinMaxADC()
{
  for ( int i = 0; i < ADC_COUNT; i++)
  {
    int v = ADC[i];
    if ( ADCMidMin[i] > v ) ADCMidMin[i] = v;
    if ( ADCMidMax[i] < v ) ADCMidMax[i] = v;
  }
}

//=====================================================================
//=====================================================================
bool TXInput::isCenterCalibrationSuccessfull()
{
  for ( int i = 0; i < ADC_COUNT; i++)
  {
    if ( (ADCMidMax[i] - ADCMidMin[i] ) > 500 ) return false;
  }

  Serial.println("Middle Min, Max:");
  Serial.printf( "%04d %04d %04d %04d\n", ADCMidMin[0]>>2, ADCMidMin[1]>>2, ADCMidMin[2]>>2, ADCMidMin[3]>>2 );
  Serial.printf( "%04d %04d %04d %04d\n", ADCMidMax[0]>>2, ADCMidMax[1]>>2, ADCMidMax[2]>>2, ADCMidMax[3]>>2 );

  for ( int i = 0; i < ADC_COUNT; i++)
  {
    int d = ADCMidMax[i] - ADCMidMin[i];
    ADCMidMin[i] -= d/10 + 4;
    ADCMidMax[i] += d/10 + 4;

    ADCMidMin[i] >>= 2;
    ADCMidMax[i] >>= 2;
  }

  Serial.println("Middle Min, Max, delta, adjusted:");
  Serial.printf( "%04d %04d %04d %04d\n", ADCMidMin[0], ADCMidMin[1], ADCMidMin[2], ADCMidMin[3] );
  Serial.printf( "%04d %04d %04d %04d\n", ADCMidMax[0], ADCMidMax[1], ADCMidMax[2], ADCMidMax[3] );
  Serial.printf( "%04d %04d %04d %04d\n", (ADCMidMax[0] - ADCMidMin[0]), (ADCMidMax[1] - ADCMidMin[1]),  (ADCMidMax[2] - ADCMidMin[2]), (ADCMidMax[3] - ADCMidMin[3]));

  return true;
}

//=====================================================================
//=====================================================================
void TXInput::loop(uint32_t t)
{
  this->readADC(t);
  this->readButtons(t);
}


