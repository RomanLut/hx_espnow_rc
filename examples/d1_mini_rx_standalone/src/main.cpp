#include <Arduino.h>
#include "HX_ESPNOW_RC_Slave.h"
#include "rx_config.h"

#include <servo.h>

HXRCSlave hxrcSlave;

unsigned long lastStats = millis();
unsigned long telemetryTime = millis();
unsigned long analogReadTime = millis();

bool servoOutputsAttached = false;
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
void attachServoOutputs()
{
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
    if ( !servoOutputsAttached )
    {
      attachServoOutputs();
      servoOutputsAttached = true;
    }

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
          break;
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
  analogWriteFreq( 8192);
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
void updatePWMOutputs()
{
  bool fs = hxrcSlave.getReceiverStats().isFailsafe();

  HXRCChannels channels = hxrcSlave.getChannels();

  for (uint8_t i = 0; i < TOTAL_CHANNELS; i++ )
  {
    if (pwmPins[i] != NOPIN)
    {
      if ( fs ) 
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
  WiFi.softAP("hxrct", NULL, USE_WIFI_CHANNEL);

  calibrateESCs();
  attachPWMPins();
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

  if ( millis() > analogReadTime )
  {
    //should not read ADC with high frequency. It will kill Wifi.
    analogReadTime = millis() + 1000;
    hxrcSlave.setA1( analogRead(0) );
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

}