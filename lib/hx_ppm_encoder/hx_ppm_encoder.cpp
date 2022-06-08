#include "hx_ppm_encoder.h"

#if defined(ESP8266)
#include <esp8266_peri.h>
#elif defined(ESP32)
#endif

#define MAX_PPM_CHANNELS_COUNT 16

#define PPM_PULSE_LENGTH_US 400
#define PPM_PAUSE_LENGTH_US 6000  //5000...20000

#if defined(ESP8266)

#define TICKS_PER_US (F_CPU / 16 / 1000000)
#define US_TO_TICKS(us) ((us)* TICKS_PER_US)

#elif defined(ESP32)

static hw_timer_t *timer = NULL;
static portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

#endif

static uint8_t tx_pin;
static uint8_t tx_pin_mask;
static uint8_t channelsCount;
static int PPMFrameLengthUS;

static volatile bool readyFailsafe = true;
static volatile bool failsafe = true;

static volatile int outChannelsIndex = 0;

static uint16_t outChannelValues[2][MAX_PPM_CHANNELS_COUNT];
static uint16_t channelValues[MAX_PPM_CHANNELS_COUNT];

//=====================================================================
//=====================================================================
enum ppmState_e
{
    PPM_STATE_PULSE,
    PPM_STATE_FILL,
    PPM_STATE_SYNC,
    PPM_STATE_FAILSAFE
};

//https://sourceforge.net/p/arduinorclib/wiki/PPM%20Signal/
//https://www.esp8266.com/viewtopic.php?f=6&t=19867

#if defined(ESP8266)
//=====================================================================
//=====================================================================
void IRAM_ATTR onTimerISR()
{
    static uint8_t ppmState = PPM_STATE_PULSE;
    static uint8_t ppmChannelIndex = 0;
    static int usedFrameLengthUS = 0;

    switch ( ppmState )
    {
    case  PPM_STATE_FAILSAFE:
        if ( !readyFailsafe ) 
        {
            ppmState = PPM_STATE_PULSE;
            outChannelsIndex ^= 1;
        }
        timer1_write(US_TO_TICKS(40000));   
        break;

    case PPM_STATE_PULSE:
        //digitalWrite(tx_pin, HIGH);
        GPOS = tx_pin_mask;
        timer1_write(US_TO_TICKS(PPM_PULSE_LENGTH_US));   
        ppmState = PPM_STATE_FILL;
        break;

    case PPM_STATE_FILL:
        //digitalWrite(tx_pin, LOW);
        GPOC = tx_pin_mask;

        ppmState = PPM_STATE_PULSE;

        if (ppmChannelIndex == channelsCount)
        {
            timer1_write(US_TO_TICKS(PPMFrameLengthUS - usedFrameLengthUS));   
      
            ppmChannelIndex = 0;
            usedFrameLengthUS = 0;

            if ( readyFailsafe )
            {
                ppmState = PPM_STATE_FAILSAFE;
            }
            else
            {
                outChannelsIndex ^= 1;
            }
        }
        else
        {
            uint16_t currentChannelValue = outChannelValues[outChannelsIndex][ppmChannelIndex];
            timer1_write(US_TO_TICKS(currentChannelValue - PPM_PULSE_LENGTH_US));   
            usedFrameLengthUS += currentChannelValue;
            ppmChannelIndex++;
        }
        break;
    }
}
#elif defined(ESP32)
//=====================================================================
//=====================================================================

//https://quadmeup.com/how-to-generate-ppm-signal-with-esp32-and-arduino/
//https://github.com/ps-after-hours/esp32_ppm_output/blob/main/esp32_ppm_output.ino
void IRAM_ATTR onTimerISR()
{
    static volatile uint8_t ppmState = PPM_STATE_PULSE;
    static volatile uint8_t ppmChannelIndex = 0;
    static volatile int usedFrameLengthUS = 0;

    portENTER_CRITICAL(&timerMux);

    switch ( ppmState ) 
    {
    case PPM_STATE_FAILSAFE:
        if ( !readyFailsafe ) 
        {
            ppmState = PPM_STATE_PULSE;
            outChannelsIndex ^= 1;
        }
        timerAlarmWrite(timer,40000, true);   
        break;

    case PPM_STATE_PULSE:
        digitalWrite(tx_pin, HIGH);
        timerAlarmWrite(timer,PPM_PULSE_LENGTH_US, true);   
        ppmState = PPM_STATE_FILL;
        break;
    
    case PPM_STATE_FILL:
        digitalWrite(tx_pin, LOW);

        ppmState = PPM_STATE_PULSE;

        if (ppmChannelIndex == channelsCount)
        {
            timerAlarmWrite(timer,PPMFrameLengthUS - usedFrameLengthUS, true);   
      
            ppmChannelIndex = 0;
            usedFrameLengthUS = 0;

            if ( readyFailsafe )
            {
                ppmState = PPM_STATE_FAILSAFE;
            }
            else
            {
                outChannelsIndex ^= 1;
            }
        }
        else
        {
            uint16_t currentChannelValue = outChannelValues[outChannelsIndex][ppmChannelIndex];
            timerAlarmWrite(timer,currentChannelValue - PPM_PULSE_LENGTH_US, true);   
            usedFrameLengthUS += currentChannelValue;
            ppmChannelIndex++;
        }
        break;
    }

  portEXIT_CRITICAL(&timerMux);
}
#endif


//=====================================================================
//=====================================================================
HXPPMEncoder::HXPPMEncoder()
{
}

//=====================================================================
//=====================================================================
void HXPPMEncoder::init( uint8_t _channelsCount, uint8_t _tx_pin )
{
    tx_pin = _tx_pin;
    tx_pin_mask = 1 << _tx_pin;
    channelsCount = _channelsCount;
    PPMFrameLengthUS = _channelsCount * 2000 + PPM_PAUSE_LENGTH_US; 

    for ( int i = 0; i < MAX_PPM_CHANNELS_COUNT; i++)
    {
        outChannelValues[0][i] = 1000;
        outChannelValues[1][i] = 1000;
        channelValues[i] = 1000;
    }

    pinMode(tx_pin, OUTPUT);
    digitalWrite(tx_pin, LOW);

#if defined(ESP8266)
    timer1_attachInterrupt(onTimerISR);
    timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE); //TIM_DIV16 => 80 MHz / 16 => 5 MHz or .2 microseconds
    timer1_write(US_TO_TICKS(12000));       
#elif defined(ESP32)
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimerISR, true);
    timerAlarmWrite(timer, 12000, true);
    timerAlarmEnable(timer);
#endif

}

//=====================================================================
//=====================================================================
void HXPPMEncoder::setFailsafe( bool _failsafe )
{
    failsafe = _failsafe;
}

//=====================================================================
//=====================================================================
void IRAM_ATTR HXPPMEncoder::commit()
{

#if defined(ESP8266)
    noInterrupts(); 
#elif defined(ESP32)
    portENTER_CRITICAL(&timerMux);
#endif

  int p = outChannelsIndex ^ 1;
  for ( int i = 0; i < channelsCount; i++)
  {
      outChannelValues[p][i] = channelValues[i];
  }
  readyFailsafe = failsafe;

#if defined(ESP8266)
  interrupts(); 
#elif defined(ESP32)
  portEXIT_CRITICAL(&timerMux);
#endif

}

//=====================================================================
//=====================================================================
//input value is in range 1000..2000
void HXPPMEncoder::setChannelValue( uint8_t index, uint16_t value ) 
{
    if ( index >= MAX_PPM_CHANNELS_COUNT ) return;
    channelValues[index] = constrain( value, 1000, 2000 );
}

