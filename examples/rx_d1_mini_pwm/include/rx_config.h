#pragma once

/* Wemos D1 Mini
            _________
           |  -.-._. |  
     /RST =o         o= TX GPIO1
     ADC0 =o         o= RX GPIO3
GPIO16 D0 =o         o= D1 GPIO5
GPIO14 D5 =o         o= D2 GPIO4
GPIO12 D6 =o         o= D3 GPIO0
GPIO13 D7 =o         o= D4 GPIO2
GPIO15 D8 =o         o= GND
     3.3V =o         o= 5V
           |___USB___| 

https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/ 

SERVO_PINS - Pins for SERVO (or ESC) output ( 1-2ms 50Hz pulses ). GPIO id for each RC channel. Set to NOPIN to skip channel.

CALIBRATE_ESC_PINS - whether to run ESC calibration on pins on boot. Applicable to SERVO_PINS only.

PWM_PINS - Pins for PWM output (OUTPUT_PWM_FREQ_HZ PWM, duty cycle 0-100%). GPIO id for each RC channel. Set to NOPIN to skip channel.

DISCRETE_PINS - Pins for discrete output: 1000 -> 0, >1250-> 1. GPIO id for each RC channel. Set to NOPIN to skip channel.

MOTOR_BEEPER_CH - channel to enable beep with PWM mottor. Value >1250 - motors with zero throttle value will start beeping. 

HDRIVER_PINS - H-BRIDGE control, f.e. l298n, mx1919, mx 619e -  2 pins for each channel

H-bridge channel mapping:
- RC value < 1500-deadband -> reverse; > 1500+deadband -> forward; near 1500 -> brake/stop
- Duty is proportional to |value-1500|

Failsafe:
 Servo pins - no pulses
 PWM pins - no pulses
 Discrete pins - keeps last state

 ADC0 can be connected to battery voltage with divider which outputs 3.2V max.
 Recommended maximum value is 2.5V because higher values are not linear in esp8266 ADC.
*/

#define NOPIN 255
//#define DEBUG_LOOP_PIN  0

//beep (with brushed motors) after period of inactivity (PWM outputs low)
#define IDLE_BEEP_DELAY_MS (5*60*1000)   

//beep (with brushed motors) in failsafe
#define FAILSAFE_BEEP_DELAY_MS (5*1000)   

//beep frequency
#define BEEP_FREQ 900

//PWM duty value to beep with brushed motors
//idle beep
#define BEEP_DUTY_VALUE 15
//high volume beep duty value, used when MOTOR_BEEPER_CH is enabled.
//This value can turn motors a little.
#define BEEP_DUTY_VALUE_HIGH 35

//if channel value is less then PWM_CH_MIN, pwm output is zero - for PWM outputs
#define PWM_CH_MIN 1050 

//deadband for center position, f.e. 100 => 1400...1600 is dead zone - ofr H_BRIDGE output
#define H_BRIDGE_DEADBAND_US   100

//PWM frequency for H-BRIDGE and PWM outputs
#define OUTPUT_PWM_FREQ_HZ    8192


//if adc read smaler values for period BATTERY_THRESHOLD_PERIOD_MS, all outputs will be disabled (low voltage protection).
//Zero means low battery protection is disabled.
#define BATTERY_THRESHOLD  0
#define BATTERY_THRESHOLD_PERIOD_MS 500

//=============================================================================
//Receiver binding
#define USE_WIFI_CHANNEL 3
#define USE_KEY 0 

//Receiver configuration 1: AETR brushless plane
//==========================================================================================
//4 servo outputs: D5,D6,D7,D8 for RC channels 1,2,3,4
//ESC calibration on channel 3 
//3 discrete outputs: D1, D2, D0, D3 - for RC channels 5,6,7,8
//A0 - ADC input

//#define TOTAL_CHANNELS   8
//#define SERVO_PINS {14 /*D5*/, 12 /*D6*/, 13 /*D7*/, 15 /*D8*/, NOPIN, NOPIN, NOPIN, NOPIN}
//#define CALIBRATE_ESC_PINS {false, false, false, true, false, false, false, false}
//#define PWM_PINS {NOPIN, NOPIN, NOPIN, NOPIN, NOPIN, NOPIN, NOPIN, NOPIN}
//#define DISCRETE_PINS {NOPIN, NOPIN, NOPIN, NOPIN, 5 /*D1*/, 4 /*D2*/, 16 /*D0*/, 0 /* D3 */}
//#define HDRIVER_PINS { {NOPIN, NOPIN}, {NOPIN, NOPIN}, {NOPIN, NOPIN}, {NOPIN, NOPIN}, {NOPIN, NOPIN}, {NOPIN, NOPIN}, {NOPIN, NOPIN}, {NOPIN, NOPIN} }
//#define MOTOR_BEEPER_CH -1
//==========================================================================================


//Receiver configuration 2: AETR brushed plane
//==========================================================================================
//3 servo outputs: D5,D6,D8 for RC channels 1,2,4
//1 PWM output for RC channel 3
//3 discrete outputs: D1, D2, D0, D3 - for RC channels 5,6,7,8
//A0 - ADC input


//#define TOTAL_CHANNELS   9
//#define SERVO_PINS {14 /*D5*/, 12 /*D6*/, 13 /*D7*/, 15 /*D8*/, NOPIN, NOPIN, NOPIN, NOPIN, NOPIN}
//#define CALIBRATE_ESC_PINS {false, false, false, false, false, false, false, false, false}
//#define PWM_PINS {NOPIN, NOPIN, 13 /*D7*/, NOPIN, NOPIN, NOPIN, NOPIN, NOPIN, NOPIN}
//#define DISCRETE_PINS {NOPIN, NOPIN, NOPIN, NOPIN, 5 /*D1*/, 4 /*D2*/, 16 /*D0*/, 0 /* D3 */, NOPIN}
//#define HDRIVER_PINS { {NOPIN, NOPIN}, {NOPIN, NOPIN}, {NOPIN, NOPIN}, {NOPIN, NOPIN}, {NOPIN, NOPIN}, {NOPIN, NOPIN}, {NOPIN, NOPIN}, {NOPIN, NOPIN}, {NOPIN, NOPIN} }
//#define MOTOR_BEEPER_CH 8 //zero-based CH number
//==========================================================================================


//Receiver configuration 3: AETT brushed twin motor plane or wing
//==========================================================================================
//2 servo outputs: D5,D6,D8 for RC channels 1,2
//2 PWM outputs for RC channels 3,4
//3 discrete outputs: D1, D2, D0, D3 - for RC channels 5,6,7,8
//A0 - ADC input

#define TOTAL_CHANNELS   9
#define SERVO_PINS {14 /*D5*/, 12 /*D6*/, NOPIN, NOPIN, NOPIN, NOPIN, NOPIN, NOPIN, NOPIN}
#define CALIBRATE_ESC_PINS {false, false, false, false, false, false, false, false, false}
#define PWM_PINS {NOPIN, NOPIN, 13 /*D7*/, 15 /*D8*/, NOPIN, NOPIN, NOPIN, NOPIN, NOPIN}
#define DISCRETE_PINS {NOPIN, NOPIN, NOPIN, NOPIN, 5 /*D1*/, 4 /*D2*/, 16 /*D0*/, 0 /* D3 */, NOPIN}
#define HDRIVER_PINS { {NOPIN, NOPIN}, {NOPIN, NOPIN}, {NOPIN, NOPIN}, {NOPIN, NOPIN}, {NOPIN, NOPIN}, {NOPIN, NOPIN}, {NOPIN, NOPIN}, {NOPIN, NOPIN}, {NOPIN, NOPIN} }
#define MOTOR_BEEPER_CH 8 //zero-based CH number
//==========================================================================================

//Notes: 
//5 - D1 - SCL, 4 - D2 - SDA
//13,15 - alternative UART1
//2 - D4 - LED
//16 - no pwm or interrupts possible
