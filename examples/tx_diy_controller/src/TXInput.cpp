#include "TXInput.h"
#include "ModeBLEGamepad.h"
#include "ErrorLog.h"
#include "AudioManager.h"

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
  this->lastAdditiveProcessing = millis();

  this->lastButtonsState = 0;
  this->buttonPressEvents = 0;

  this->resetLastChannelValues();

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
    buttonData[i] &= 15;
  }

  uint16_t buttonsState = 0;
  for ( int i = 0; i < BUTTONS_COUNT; i++ )
  {
    if ( this->isButtonPressed(i) )
    {
      buttonsState |= 1 << i;
    }
  }
  this->buttonPressEvents |= (this->lastButtonsState ^ buttonsState) & buttonsState;  
  this->lastButtonsState = buttonsState;
}

//=====================================================================
//=====================================================================
void TXInput::resetLastChannelValues()
{
  for ( int i =0; i < HXRC_CHANNELS_COUNT; i++ )  
  {
    this->lastChannelValue[i] = 1000;
  }
}

//=====================================================================
//=====================================================================
int16_t TXInput::mapADCValue( int ADCId )
{
  int v = (this->ADC[ADCId] + 2) >> 2;
  if ( v < ADCMidMin[ADCId] )
  {
      v = map( v, ADCMidMin[ADCId], ADCMin[ADCId], 1500, 1000);
      return v < 1000 ? 1000 : v;
  }
  else if ( v > ADCMidMax[ADCId] )
  {
      v = map( v, ADCMidMax[ADCId], ADCMax[ADCId], 1500, 2000);
      return v > 2000 ? 2000 : v;
  }
  return 1500;
}

//=====================================================================
//=====================================================================
int16_t TXInput::mapButtonValue( int buttonId )
{
  return ( buttonData[buttonId] == 0 ) ? 2000: 1000;
}

//=====================================================================
//=====================================================================
void TXInput::getChannelValuesDefault( out HXChannels* channelValues )
{
  channelValues-> isFailsafe = false;
  for ( int i = 0; i < HXRC_CHANNELS_COUNT; i++)
  {
    channelValues->channelValue[i] = 1000;
  }

  for ( int i = 0; i < ADC_COUNT; i++ )
  {
    channelValues->channelValue[i] = this->mapADCValue(i);
  }

  for ( int i = 0; i < BUTTONS_COUNT; i++ )
  {
    channelValues->channelValue[BLE_GAMEPAD_AXIS_COUNT+i] = ( buttonData[i] == 0 ) ? 2000: 1000;
  }
}

//=====================================================================
//=====================================================================
int16_t TXInput::chClamp(int16_t value)
{
  if ( value < 1000) return 1000;
  if ( value > 2000) return 2000;
  return value;
}

//=====================================================================
//=====================================================================
int16_t TXInput::chMul10(int16_t value, int mul)
{
  return ((int32_t)value - 1500)  * mul / 10 + 1500;
}

//=====================================================================
//=====================================================================
int16_t TXInput::chAdditive(int16_t value, const char* axisName, int speed, int32_t dT)
{

  value = value  + (this->getAxisValueByName(axisName) - 1500) * dT * speed / 10000;

  if ( value < 1000) return 1000;
  if ( value > 2000) return 2000;
  return value;
}

//=====================================================================
//=====================================================================
int TXInput::getAxisIdByName(const char* parm)
{
  if ( strcmp(parm, "LEFT_STICK_X") == 0 )
  {
    return LEFT_STICK_X_ID;
  }  
  else if ( strcmp(parm, "LEFT_STICK_Y") == 0 )
  {
    return LEFT_STICK_Y_ID;
  }  
  else if ( strcmp(parm, "RIGHT_STICK_X") == 0 )
  {
    return RIGHT_STICK_X_ID;
  }  
  else if ( strcmp(parm, "RIGHT_STICK_Y") == 0 )
  {
    return RIGHT_STICK_Y_ID;
  }  
  else
  {
      for ( int i = 0; i < ADC_COUNT; i++)
      {
        char axisName[32];
        sprintf( axisName, "AXIS%d", i);
        if ( strcmp(parm, axisName) == 0 )
        {
          return i;
        }  
      }
      ErrorLog::instance.writeOnce("Invalid axis name:");
      ErrorLog::instance.writeOnce(parm);
      ErrorLog::instance.writeOnce("\n");
      ErrorLog::instance.disableWriteOnce();
  }
  return 0;
}

//=====================================================================
//=====================================================================
int16_t TXInput::getAxisValueByName(const char* parm)
{
  return this->mapADCValue( this->getAxisIdByName(parm) );
}

//=====================================================================
//=====================================================================
int TXInput::getButtonIdByName( const char* parm)
{
  if ( strcmp(parm, "LEFT_BUMPER") == 0 )
  {
    return LEFT_BUMPER_ID;
  }  
  else if ( strcmp(parm, "RIGHT_BUMPER") == 0 )
  {
    return RIGHT_BUMPER_ID;
  }  
  else
  {
      for ( int i = 0; i < BUTTONS_COUNT; i++)
      {
        char buttonName[32];
        sprintf( buttonName, "BUTTON%d", i);
        if ( strcmp(parm, buttonName) == 0 )
        {
          return i;
        }  
      }
      ErrorLog::instance.writeOnce("Invalid button name:");
      ErrorLog::instance.writeOnce(parm);
      ErrorLog::instance.writeOnce("\n");
      ErrorLog::instance.disableWriteOnce();
  }
  return 0;
}

//=====================================================================
//=====================================================================
int16_t TXInput::getButtonValueByName( const char* parm)
{
  return this->mapButtonValue( this->getButtonIdByName(parm) );
}

//=====================================================================
//=====================================================================
bool TXInput::hasButtonPressEventByName( const char* parm )
{
  return this->buttonPressEvents & (1 << this->getButtonIdByName(parm));
}

//=====================================================================
//=====================================================================
void TXInput::getChannelValuesMapping( HXChannels* channelValues, const JsonArray& mapping, bool startup)
{
  channelValues-> isFailsafe = false;

  uint32_t t = millis();
  int32_t dT = t - this->lastAdditiveProcessing;
  if ( dT > 10 )  //process every 10ms to avoid to small values in integer math
  {
    this->lastAdditiveProcessing = t;
    if ( dT>10) dT = 0;
  }
  else
  {
    dT = 10;
  }

  for ( int i = 0; i < mapping.size(); i++)
  {
    const JsonVariant& action = mapping[i];

    const char* event = action["event"] | "";
    int channelIndex = (action["channel"] | -1) - 1;

    if (channelIndex < 0 || channelIndex >= HXRC_CHANNELS_COUNT-1)
    {
      ErrorLog::instance.writeOnce("Invalid channelIndex in mapping\n");
      ErrorLog::instance.disableWriteOnce();
      return;
    }

    bool run = false;
    if ( strcmp(event, "STARTUP") == 0)
    {
      run = startup;
    }
    else if ( strcmp(event, "ALWAYS") == 0)
    {
      run = !startup;
    }
    else if ( strcmp(event, "CHANNEL_EQUAL_1000") == 0)
    {
      run = !startup && (channelValues->channelValue[channelIndex] == 1000) && (this->lastChannelValue[channelIndex] != 1000);
    }
    else if ( strcmp(event, "CHANNEL_EQUAL_1333") == 0)
    {
      run = !startup && (channelValues->channelValue[channelIndex] == 1333) && (this->lastChannelValue[channelIndex] != 1333);
    }
    else if ( strcmp(event, "CHANNEL_EQUAL_1500") == 0)
    {
      run = !startup && (channelValues->channelValue[channelIndex] == 1500) && (this->lastChannelValue[channelIndex] != 1500);
    }
    else if ( strcmp(event, "CHANNEL_EQUAL_1666") == 0)
    {
      run = !startup && (channelValues->channelValue[channelIndex] == 1666) && (this->lastChannelValue[channelIndex] != 1666);
    }
    else if ( strcmp(event, "CHANNEL_EQUAL_2000") == 0)
    {
      run = !startup && (channelValues->channelValue[channelIndex] == 2000) && (this->lastChannelValue[channelIndex] != 2000);
    }
    else
    {
      ErrorLog::instance.writeOnce("Invalid event in mapping:");
      ErrorLog::instance.writeOnce(event);
      ErrorLog::instance.writeOnce("\n");
      ErrorLog::instance.disableWriteOnce();
    }

    if ( run )
    {
      const char* op = action["op"] | "";      
      const char* parm = action["parm"] | "";      

      if ( strcmp(op, "AXIS") == 0)
      {
        channelValues->channelValue[channelIndex] = this->getAxisValueByName(parm);
      }
      else if ( strcmp(op, "BUTTON") == 0)
      {
        channelValues->channelValue[channelIndex] = this->getButtonValueByName(parm);
      }
      else if ( strcmp(op, "TRIGGER") == 0)
      {
        if ( this->hasButtonPressEventByName(parm) )
        {
          channelValues->channelValue[channelIndex] = ( channelValues->channelValue[channelIndex] == 1000) ? 2000: 1000;
          //Serial.print(channelValues->channelValue[channelIndex]);
        }
      }
      else if ( strcmp(op, "CONSTANT") == 0)
      {
        channelValues->channelValue[channelIndex] = action["parm"] | 0;
      }
      else if ( strcmp(op, "SWITCH3") == 0)
      {
        if ( this->hasButtonPressEventByName(parm) )
        {
          if ( channelValues->channelValue[channelIndex] < 1500 )
          {
            channelValues->channelValue[channelIndex] = 1500;
          }
          else if ( channelValues->channelValue[channelIndex] < 2000 )
          {
            channelValues->channelValue[channelIndex] = 2000;
          }
          else
          {
            channelValues->channelValue[channelIndex] = 1000;              
          }
        }
      }
      else if ( strcmp(op, "SWITCH4") == 0)
      {
        if ( this->hasButtonPressEventByName(parm) )
        {
          if ( channelValues->channelValue[channelIndex] < 1333 )
          {
            channelValues->channelValue[channelIndex] = 1333;
          }
          else if ( channelValues->channelValue[channelIndex] < 1666 )
          {
            channelValues->channelValue[channelIndex] = 1666;
          }
          else if ( channelValues->channelValue[channelIndex] < 2000 )
          {
            channelValues->channelValue[channelIndex] = 2000;
          }
          else
          {
            channelValues->channelValue[channelIndex] = 1000;              
          }
        }
      }
      else if ( strcmp(op, "ADD") == 0)
      {
        channelValues->channelValue[channelIndex] = channelValues->channelValue[channelIndex] + (action["parm"] | 0);
      }
      else if ( strcmp(op, "MUL") == 0)
      {
        channelValues->channelValue[channelIndex] = this->chMul10( channelValues->channelValue[channelIndex], (action["parm"] | 1 )  );
      }
      else if ( strcmp(op, "ADDITIVE") == 0)
      {
        channelValues->channelValue[channelIndex] = this->chAdditive( channelValues->channelValue[channelIndex], action["parm"] | "", (action["speed"] | 1 ), dT  );
      }
      else if ( strcmp(op, "SOUND") == 0)
      {
        AudioManager::instance.play(String(parm), AUDIO_GROUP_NONE);
      }
      else
      {
        ErrorLog::instance.writeOnce("Invalid operation:");
        ErrorLog::instance.writeOnce(op);
        ErrorLog::instance.writeOnce("\n");
        ErrorLog::instance.disableWriteOnce();
      }
    }

  }

  for ( int i = 0; i < HXRC_CHANNELS_COUNT; i++ )
  {
    channelValues->channelValue[i] = this->chClamp(channelValues->channelValue[i]);
  }

}


//=====================================================================
//=====================================================================
void TXInput::getChannelValues( HXChannels* channelValues )
{
  JsonDocument* profile = TXProfileManager::instance.getCurrentProfile();

  if (profile)
  {
    const JsonArray& mapping = (*profile)["mapping"];
    if ( mapping.size() > 0 )
    {
      bool startup = false;
      int profileIndex = TXProfileManager::instance.getCurrentProfileIndex();
      if ( this->lastProfileIndex != profileIndex )
      {
        this->lastProfileIndex = profileIndex;
        this->resetLastChannelValues();
        this->buttonPressEvents = 0;
        startup = true;
      }

      memcpy( channelValues->channelValue, this->lastChannelValue, HXRC_CHANNELS_COUNT * sizeof(uint16_t));

      this->getChannelValuesMapping( channelValues, mapping, startup);

      this->buttonPressEvents = 0;
      memcpy( this->lastChannelValue, channelValues->channelValue, HXRC_CHANNELS_COUNT * sizeof(uint16_t));

      return;
    }
  }

  this->getChannelValuesDefault( channelValues );
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
bool TXInput::isButtonPressed(uint8_t buttonId)
{
  if ( this->buttonData[buttonId] == 0 ) 
  {
    return true;
  }
  else if ( this->buttonData[buttonId] == 15 ) 
  {
    return false;
  }
  else
  {
    return this->lastButtonsState && (1 << buttonId);
  }
}

//=====================================================================
//=====================================================================
bool TXInput::isButtonUnPressed(uint8_t buttonId)
{
  return !this->isButtonPressed(buttonId);
}

//=====================================================================
//=====================================================================
bool TXInput::isStickMin(uint8_t stickId)
{
  return ADC[stickId]>>2 < ( ADCMidMin[stickId]  + ADCMin[stickId] ) / 2;
}

//=====================================================================
//=====================================================================
bool TXInput::isStickMiddle(uint8_t stickId)
{
  return ( ADC[stickId]>>2 > (ADCMin[stickId] + ADCMidMin[stickId]*3)/4) && ( ADC[stickId]>>2 < (ADCMidMax[stickId]*3 + ADCMax[stickId])/4 );
}

//=====================================================================
//=====================================================================
bool TXInput::isStickMax(uint8_t stickId)
{
  return ADC[stickId]>>2 >  ( ADCMidMax[stickId]  + ADCMax[stickId] ) / 2;
}


//=====================================================================
//=====================================================================
void TXInput::loop(uint32_t t)
{
  this->readADC(t);
  this->readButtons(t);
}


