#include <Arduino.h>
#include "HX_ESPNOW_RC_Slave.h"
#include "rx_config.h"

#include <servo.h>

HXRCSlave hxrcSlave;

unsigned long lastStats = millis();
unsigned long telemetryTime = millis();

Servo channelServo[TOTAL_CHANNELS];
uint8_t channelPin[TOTAL_CHANNELS] = CHANNEL_PINS;

bool attached = false;

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
    //TODO: write SPORT packets
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
void detach()
{
  for (uint8_t i = 0; i < TOTAL_CHANNELS; i++ )
  {
    channelServo[i].detach();
    digitalWrite(channelPin[i], LOW );  
  }
}

//=====================================================================
//=====================================================================
void attach()
{
  for (uint8_t i = 0; i < TOTAL_CHANNELS; i++ )
  {
    channelServo[i].attach(channelPin[i]);
  }
}

//=====================================================================
//=====================================================================
void calibrateESCs()
{
  bool flags[] = CALIBRATE_ESC_CHANNELS;

  bool anyFlagSet = false;
  for (uint8_t i = 0; i < TOTAL_CHANNELS; i++ )
  {
    anyFlagSet |= flags[i];
  }
  if ( !anyFlagSet) return;

  for (uint8_t i = 0; i < TOTAL_CHANNELS; i++ )
  {
    if ( !flags[i] ) continue;
    channelServo[i].attach(channelPin[i]);
    channelServo[i].write(2000);
  }

  delay(5000);

  for (uint8_t i = 0; i < TOTAL_CHANNELS; i++ )
  {
    if ( !flags[i] ) continue;
    channelServo[i].attach(channelPin[i]);
    channelServo[i].write(1000);
  }

  delay(1000);
}


//=====================================================================
//=====================================================================
void setup()
{
#if defined(DEBUG_LOOP_PIN)
    pinMode(DEBUG_LOOP_PIN, OUTPUT );  
    digitalWrite(DEBUG_LOOP_PIN, LOW );  
#endif

  for (uint8_t i = 0; i < TOTAL_CHANNELS; i++ )
  {
    pinMode(channelPin[i], OUTPUT );  
    digitalWrite(channelPin[i], LOW );  
  }

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
}

//=====================================================================
//=====================================================================
void setOutputs()
{
  static uint8_t counter = 0;

  if ( hxrcSlave.getReceiverStats().isFailsafe() )
  {
    if ( attached )
    {
      detach();
      attached = false;
    }
  }
  else
  {
    if ( !attached )
    {
      attach();
      attached = true;
    }

    HXRCChannels channels = hxrcSlave.getChannels();
    
    for (uint8_t i = 0; i < TOTAL_CHANNELS; i++ )
    {
      counter++;
      if (counter == TOTAL_CHANNELS) counter = 0;

      //calling Servo.writeMicroseconds() with same value continuously with high frequency will produce delay up to 20ms on each call,
      //because waveform generator will wait untill previous settins are applied.
      //so avoid redundand calls to writeMicroseconds()
	    if ( channelServo[counter].readMicroseconds() != channels.getChannelValue(counter) )
      {
	      channelServo[counter].writeMicroseconds(channels.getChannelValue(counter));

        //in any case, update up to 1 servo at a time. 20 ms per servo is quaite slow.
        break;
      }
    }

  }
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

  hxrcSlave.loop();

  if (millis() - lastStats > 1000)
  {
    lastStats = millis();
    hxrcSlave.getTransmitterStats().printStats();
    hxrcSlave.getReceiverStats().printStats();
  }

  setOutputs();

#if defined(DEBUG_LOOP_PIN)
    digitalWrite(DEBUG_LOOP_PIN, LOW );  
#endif

  //Serial.println(millis()-t);

}