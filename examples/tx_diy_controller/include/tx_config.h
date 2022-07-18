#pragma once

//Watchdog time timeout, seconds
#define WDT_TIMEOUT_SECONDS 10  

//--------------------------------------------
//UART for HC-06 module
#define HC06_INTERFACE_RX_PIN  16  //SERIAL2 RX
#define HC06_INTERFACE_TX_PIN  17  //SERIAL2 TX

//we sense HC-06 LED pin to check module connection status
#define HC06_LED_PIN 4

//--------------------------------------------
//uncomment if LEDS are inverted (connected to +3.3V)
#define LED_INVERT

#define LED1_PIN 13
#define LED2_PIN 14
#define LED3_PIN 27
#define LED4_PIN 23
//--------------------------------------------

#define SPEAKER_PIN 22  
//pin 25, 26 - occupied by I2S

//--------------------------------------------
#define AXIS_COUNT 5

#define ADC_COUNT (AXIS_COUNT + 1) //+battery pin

#define LEFT_STICK_X_ID     0   //== AXIS0
#define LEFT_STICK_Y_ID     1   //== AXIS1
#define RIGHT_STICK_X_ID    2   //== AXIS2
#define RIGHT_STICK_Y_ID    3   //== AXIS3
#define EXT_AXIS_ID         4   //== AXIS4

//--------------------------------------------

#define LEFT_STICK_X_PIN    35
#define LEFT_STICK_Y_PIN    34
#define RIGHT_STICK_X_PIN   36
#define RIGHT_STICK_Y_PIN   39
#define EXT_AXIS_PIN        32

//--------------------------------------------
#define BAT_ADC             AXIS_COUNT
#define BAT_ADC_PIN         33
//#define BAT_4P2             13600  //ADC value at 4.2V
#define BAT_LOW_HIGH        10400  //low voltage ADC value (~3.3V)
#define BAT_LOW_LOW         10280  //low voltage ADC Value - threshold

//--------------------------------------------

#define AXIS_INVERT_LIST {true, false, true, false, false};
#define ADC_PINS_LIST {LEFT_STICK_X_PIN, LEFT_STICK_Y_PIN, RIGHT_STICK_X_PIN, RIGHT_STICK_Y_PIN, EXT_AXIS_PIN, BAT_ADC_PIN};

//--------------------------------------------
#define BUTTONS_COUNT 4

#define BUTTONS_CH_BASE     AXIS_COUNT

#define LEFT_BUMPER_ID      0  //== BUTTON0 
#define RIGHT_BUMPER_ID     1  //== BUTTON1
#define LEFT_TRIGGER_ID     2  //== BUTTON2 
#define RIGHT_TRIGGER_ID    3  //== BUTTON3
//#define BUTTON4_ID          4  
//#define BUTTON5_ID          5  

#define LEFT_BUMPER_PIN     5
#define RIGHT_BUMPER_PIN    18
#define LEFT_TRIGGER_PIN    19
#define RIGHT_TRIGGER_PIN   21
//#define BUTTON4_PIN         14  // pulled up
//#define BUTTON5_PIN         15  
//--------------------------------------------

#define BUTTON_PINS_LIST { LEFT_BUMPER_PIN, RIGHT_BUMPER_PIN, LEFT_TRIGGER_PIN, RIGHT_TRIGGER_PIN/*, BUTTON4_PIN, BUTTON5_PIN*/};

//Used:
//0 - pulled up
//2 - onboard LED
//Free: 
//12 (boot fails if pooled high)