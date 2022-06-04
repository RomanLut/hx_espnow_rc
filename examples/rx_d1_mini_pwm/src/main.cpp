#include <Arduino.h>
#include "HX_ESPNOW_RC_Slave.h"
#include "rx_config.h"

#include <Servo.h>
#include <ArduinoOTA.h>

HXRCSlave hxrcSlave;

unsigned long lastStats = millis();
unsigned long telemetryTime = millis();
unsigned long analogReadTime = millis();
unsigned long failsafeStart = 0;
unsigned long idleStart = 0;
uint16_t battSum = 0;

bool servoOutputsAttached = false;
bool PWMOutputsAttached = false;
Servo servos[TOTAL_CHANNELS];

uint8_t servoPins[TOTAL_CHANNELS] = SERVO_PINS;
uint8_t pwmPins[TOTAL_CHANNELS] = PWM_PINS;
uint8_t discretePins[TOTAL_CHANNELS] = DISCRETE_PINS;


//=====================================================================
//=====================================================================
void processIncomingTelemetry()
{
  uint8_t buffer[100];

  for ( int j = 0; j < 10; j++ )
  {
    uint16_t returnedSize = hxrcSlave.getIncomingTelemetry( 100, buffer );
    if ( returnedSize == 0 ) break;

    //process incoming telemetry here
  }
}

//=====================================================================
//=====================================================================
void fillOutgoingTelemetry()
{
  unsigned long t = millis();
  if ( t - telemetryTime > 100 )
  {
    telemetryTime = t;
  
    /*
    //write outgoing teemetry here
    if ( hxrcSlave.sendOutgoingTelemetry( buffer, len ))
    {
      outgoingTelVal = v;
      rateCounter += len;
    }
    */
  }
}

//=====================================================================
//=====================================================================
void attachServoOutputs(bool force)
{
  if ( !force && servoOutputsAttached ) return;
  servoOutputsAttached = true;

  for (uint8_t i = 0; i < TOTAL_CHANNELS; i++ )
  {
    if (servoPins[i] != NOPIN)
    {
      servos[i].attach(servoPins[i]);
    }
  }
}

//=====================================================================
//=====================================================================
void detachServoOutputs()
{
  if ( !servoOutputsAttached ) return;
  servoOutputsAttached = false;

  for (uint8_t i = 0; i < TOTAL_CHANNELS; i++ )
  {
    if (servoPins[i] != NOPIN)
    {
      servos[i].detach();
      digitalWrite(servoPins[i], LOW );  
    }
  }
}

//=====================================================================
//=====================================================================
void updateServoOutputs()
{
  static uint8_t counter = 0;

  if ( hxrcSlave.getReceiverStats().isFailsafe() )
  {
    if ( servoOutputsAttached )
    {
      detachServoOutputs();
      servoOutputsAttached = false;
    }
  }
  else
  {
    bool b = servoOutputsAttached;
    attachServoOutputs(false);

    HXRCChannels channels = hxrcSlave.getChannels();
    
    for (uint8_t i = 0; i < TOTAL_CHANNELS; i++ )
    {
      counter++;
      if (counter == TOTAL_CHANNELS) counter = 0;

      if (servoPins[counter] != NOPIN)
      {
        //calling Servo.writeMicroseconds() continuously will produce delay up to 20ms on each call,
        //because waveform generator will wait until previous settings are applied.
        //so avoid redundand calls to writeMicroseconds()
        if ( servos[counter].readMicroseconds() != channels.getChannelValue(counter) )
        {
          servos[counter].writeMicroseconds(channels.getChannelValue(counter));

          //in any case, update up to 1 servo at a time. 20 ms per servo is quite slow.
          //but if we just attached all servos after failsave (b == false), write all.
          if ( b ) break;
        }
      }
    }
  }
}

//=====================================================================
//=====================================================================
void calibrateESCs()
{
  bool flags[] = CALIBRATE_ESC_PINS;

  bool anyFlagSet = false;
  for (uint8_t i = 0; i < TOTAL_CHANNELS; i++ )
  {
    anyFlagSet |= flags[i];
  }
  if ( !anyFlagSet) return;

  for (uint8_t i = 0; i < TOTAL_CHANNELS; i++ )
  {
    if ( !flags[i] ) continue;
    servos[i].attach(servoPins[i]);
    servos[i].write(2000);
  }

  delay(5000);

  for (uint8_t i = 0; i < TOTAL_CHANNELS; i++ )
  {
    if ( !flags[i] ) continue;
    servos[i].attach(servoPins[i]);
    servos[i].write(1000);
  }

  delay(1000);
}

//=====================================================================
//=====================================================================
void attachPWMPins()
{
  if ( PWMOutputsAttached ) return;
  PWMOutputsAttached = true;
  
  analogWriteFreq( 8192 );
  analogWriteResolution(10);
  
  for (uint8_t i = 0; i < TOTAL_CHANNELS; i++ )
  {
    if ((pwmPins[i] != NOPIN))
    {
      analogWrite( pwmPins[i], 0);
    }
  }
}

//=====================================================================
//=====================================================================
void attachPWMPinsBeep( bool force )
{
  if ( !force && !PWMOutputsAttached ) return;
  PWMOutputsAttached = false;
  
  analogWriteFreq( BEEP_FREQ );
  analogWriteResolution(10);
  
  for (uint8_t i = 0; i < TOTAL_CHANNELS; i++ )
  {
    if ((pwmPins[i] != NOPIN))
    {
      analogWrite( pwmPins[i], 0);
    }
  }
}

//=====================================================================
//=====================================================================
void writeBeepDutyValue( bool b, bool highVolume )
{
  for (uint8_t i = 0; i < TOTAL_CHANNELS; i++ )
  {
    if (pwmPins[i] != NOPIN)
    {
      analogWrite(pwmPins[i], b ? (highVolume? BEEP_DUTY_VALUE_HIGH : BEEP_DUTY_VALUE):0);
    }
  }
}

//=====================================================================
//=====================================================================
void processBeep()
{
  bool b = false;
  bool high = false;

  bool fs = hxrcSlave.getReceiverStats().isFailsafe();
  
  if ( hxrcSlave.getReceiverStats().isFailsafe() ) 
  {
    if ( failsafeStart == 0 ) 
    {
      failsafeStart = millis();
    } 

    if ((millis() - failsafeStart) >= FAILSAFE_BEEP_DELAY_MS)
    {
      if ( (millis() % 3000) < 100 ) 
      {
        b = true;
      }
    }
  }
  else
  {
    failsafeStart = 0;
  }

  if ( (idleStart != 0) && ((millis() - idleStart) > IDLE_BEEP_DELAY_MS))
  {
      if ( (millis() % 3000) < 100 ) 
      {
        b |= true;
      }
  }

  if ( !fs && (MOTOR_BEEPER_CH >= 0) )
  {
    HXRCChannels channels = hxrcSlave.getChannels();
    if ( channels.getChannelValue(MOTOR_BEEPER_CH) >= 1750) 
    {
      if ( (millis() % 3000) < 100 ) 
      {
        b |= true;
        high = true;
      }
    }
  }

  writeBeepDutyValue(b, high);
}

//=====================================================================
//=====================================================================
void updatePWMOutputs()
{
  bool fs = hxrcSlave.getReceiverStats().isFailsafe();
  HXRCChannels channels = hxrcSlave.getChannels();

  bool canBeep = true;

  for (uint8_t i = 0; i < TOTAL_CHANNELS; i++ )
  {
    if (pwmPins[i] != NOPIN)
    {
      if ( !fs && (channels.getChannelValue(i) >= PWM_CH_MIN )) 
      {
        canBeep = false;
        break;
      }
    }
  }

  if ( canBeep )
  {
    if ( !fs && (idleStart == 0) )
    {
      idleStart = millis(); 
    }
    attachPWMPinsBeep(false);
    processBeep();
  }
  else
  {
    idleStart = 0;

    attachPWMPins();

    for (uint8_t i = 0; i < TOTAL_CHANNELS; i++ )
    {
      if (pwmPins[i] != NOPIN)
      {
        if ( channels.getChannelValue(i) < PWM_CH_MIN ) 
        {
          analogWrite(pwmPins[i], 0 );  
        }
        else
        {
          analogWrite(pwmPins[i], map( constrain( channels.getChannelValue(i), 1000, 2000), 1000, 2000, 0, 1023 ) );  
        }
      }
    }
  }

}

//=====================================================================
//=====================================================================
void updateDiscreteOutputs()
{
  if( hxrcSlave.getReceiverStats().isFailsafe() ) return;

  HXRCChannels channels = hxrcSlave.getChannels();

  for (uint8_t i = 0; i < TOTAL_CHANNELS; i++ )
  {
    if (discretePins[i] != NOPIN)
    {
      digitalWrite(discretePins[i], channels.getChannelValue(i)>1200? HIGH : LOW );  
    }
  }
}

//=====================================================================
//=====================================================================
//write zeros to all outputs
void zeroOutputs()
{
  for (uint8_t i = 0; i < TOTAL_CHANNELS; i++ )
  {
    if ((servoPins[i] != NOPIN) || (pwmPins[i] != NOPIN) || (discretePins[i] != NOPIN))
    {
      pinMode(servoPins[i], OUTPUT );  
      digitalWrite(servoPins[i], LOW );  
    }
  }
}

//=====================================================================
//=====================================================================
void setup()
{
#if defined(DEBUG_LOOP_PIN)
    pinMode(DEBUG_LOOP_PIN, OUTPUT );  
    digitalWrite(DEBUG_LOOP_PIN, LOW );  
#endif

  zeroOutputs();

  Serial.begin(115200);
  Serial.println("Start");

  hxrcSlave.init(
      HXRCConfig(
          USE_WIFI_CHANNEL,
          USE_KEY,
          false,
          LED_BUILTIN, true));

  //REVIEW: receiver does not work if AP is not initialized?
  WiFi.softAP("hxrcr", NULL, USE_WIFI_CHANNEL);

  ArduinoOTA.begin();  

  calibrateESCs();

  attachPWMPinsBeep(true);
  writeBeepDutyValue(true, false);
  delay(100);
  writeBeepDutyValue(false, false);
  delay(100);
  writeBeepDutyValue(true, false);
  delay(100);
  writeBeepDutyValue(false, false);
  attachPWMPins();
}

//=====================================================================
//=====================================================================
//returns true if all PWM outputs are at minimum OR failsafe
bool isThrottleLow() 
{
  if ( hxrcSlave.getReceiverStats().isFailsafe() ) return true;

  HXRCChannels channels = hxrcSlave.getChannels();
  
  for (uint8_t i = 0; i < TOTAL_CHANNELS; i++ )
  {
    if (pwmPins[i] != NOPIN)
    {
      if ( channels.getChannelValue(i) >= PWM_CH_MIN ) return false;
    }
  }

  return true;
}

//=====================================================================
//=====================================================================
void loop()
{
  //unsigned long t = millis();

#if defined(DEBUG_LOOP_PIN)
    digitalWrite(DEBUG_LOOP_PIN, HIGH );  
#endif

  processIncomingTelemetry();
  fillOutgoingTelemetry();

  if ( (millis() - analogReadTime) > 500 )
  {
    //should not read ADC with high frequency. It will kill Wifi.
    analogReadTime = millis();

    //without filter, A1 = ( analogRead(0) + 2 ) >> 2

    uint16_t v = analogRead(0);
    if ( battSum == 0 ) 
    {
      battSum = v << 1;
    }
    else
    {
      battSum = battSum - (( battSum + 4 ) >> 3) + ((v + 2) >> 2);
    }

    hxrcSlave.setA1( (battSum + 4 ) >> 3);
  }

  hxrcSlave.loop();

  if (millis() - lastStats > 1000)
  {
    lastStats = millis();
    hxrcSlave.getTransmitterStats().printStats();
    hxrcSlave.getReceiverStats().printStats();
  }

  updateServoOutputs();
  updatePWMOutputs();
  updateDiscreteOutputs();

#if defined(DEBUG_LOOP_PIN)
    digitalWrite(DEBUG_LOOP_PIN, LOW );  
#endif

  //Serial.println(millis()-t);

  if ( hxrcSlave.getReceiverStats().isFailsafe() )
  {
    ArduinoOTA.handle();  
  }
}