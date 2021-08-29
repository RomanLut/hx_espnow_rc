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

SERVO_PINS - Pins for SERVO output ( 1-2ms 50Hz pulses ). GPIO id for each RC channel. Set to NOPIN to skip channel.

CALIBRATE_ESC_PINS - run ESC calibration on pins on boot. Applicable to SERVO_PINS only.

PWM_PINS - Pins for PWM output (8192Hz PWM, duty cycle 0-100%). GPIO id for each RC channel. Set to NOPIN to skip channel.

DISCRETE_PINS - Pins for discrete output: 1000 -> 0, >1250-> 1. GPIO id for each RC channel. Set to NOPIN to skip channel.

Failsafe:
 Servo pins - no pulses
 PWM pins - no pulses
 Discrete pins - keeps last state
*/

#define NOPIN 255
//#define DEBUG_LOOP_PIN  0

//=============================================================================
//Receiver binding
#define USE_WIFI_CHANNEL 3
#define USE_KEY 0 

//Receiver configuration 1: AETR brushless plane
//==========================================================================================
//4 servo outputs: D5,D6,D7,D8 for RC channels 1,2,3,4
//ESC calibration on channel 3 
//3 discrete outputs: D1, D2, D0 - for RC channels 5,6,7
//A0 - ADC input

//#define TOTAL_CHANNELS   8
//#define SERVO_PINS {14 /*D5*/, 12 /*D6*/, 13 /*D7*/, 15 /*D8*/, NOPIN, NOPIN, NOPIN, NOPIN}
//#define CALIBRATE_ESC_PINS {false, false, false, true, false, false, false, false}
//#define PWM_PINS {NOPIN, NOPIN, NOPIN, NOPIN, NOPIN, NOPIN, NOPIN, NOPIN}
//#define DISCRETE_PINS {NOPIN, NOPIN, NOPIN, NOPIN, 5 /*D1*/, 4 /*D2*/, 16 /*D0*/, 0 /* D3 */}
//==========================================================================================


//Receiver configuration 2: AETR brushed plane
//==========================================================================================
//3 servo outputs: D5,D6,D8 for RC channels 1,2,4
//1 PWM output for RC channel 3
//3 discrete outputs: D1, D2, D0 - for RC channels 5,6,7
//A0 - ADC input

//#define TOTAL_CHANNELS   8
//#define SERVO_PINS {14 /*D5*/, 12 /*D6*/, NOPIN, 15 /*D8*/, NOPIN, NOPIN, NOPIN, NOPIN}
//#define CALIBRATE_ESC_PINS {false, false, false, false, false, false, false, false}
//#define PWM_PINS {NOPIN, NOPIN, 13 /*D7*/, NOPIN, NOPIN, NOPIN, NOPIN, NOPIN}
//#define DISCRETE_PINS {NOPIN, NOPIN, NOPIN, NOPIN, 5 /*D1*/, 4 /*D2*/, 16 /*D0*/, 0 /* D3 */}
//==========================================================================================


//Receiver configuration 3: AETT brushed twin motor plane or wing
//==========================================================================================
//2 servo outputs: D5,D6,D8 for RC channels 1,2
//2 PWM outputs for RC channels 3,4
//3 discrete outputs: D1, D2, D0 - for RC channels 5,6,7
//A0 - ADC input

#define TOTAL_CHANNELS   8
#define SERVO_PINS {14 /*D5*/, 12 /*D6*/, NOPIN, NOPIN, NOPIN, NOPIN, NOPIN, NOPIN}
#define CALIBRATE_ESC_PINS {false, false, false, false, false, false, false, false}
#define PWM_PINS {NOPIN, NOPIN, 13 /*D7*/, 15 /*D8*/, NOPIN, NOPIN, NOPIN, NOPIN}
#define DISCRETE_PINS {NOPIN, NOPIN, NOPIN, NOPIN, 5 /*D1*/, 4 /*D2*/, 16 /*D0*/, 0 /* D3 */}
//==========================================================================================

//Notes: 
//5 - D1- SCL, 4 -  D2- SDA
//13,15 - alternative UART1
//2 - D4 - LED
//16 - no pwm or interrupts

