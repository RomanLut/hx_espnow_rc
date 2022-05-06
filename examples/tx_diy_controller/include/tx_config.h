#pragma once

#define WDT_TIMEOUT_SECONDS 10  

#define HC06_INTERFACE_RX_PIN  17
#define HC06_INTERFACE_TX_PIN  16

#define LED1_PIN 12
#define LED2_PIN 13
#define LED3_PIN 27
#define LED4_PIN 23

#define SPEAKER_PIN 22  

//--------------------------------------------
#define AXIS_COUNT 4
#define SLIDERS_COUNT 0 //TODO: support for sliders (non-centering axis or multiple position switch)

#define ADC_COUNT (AXIS_COUNT + SLIDERS_COUNT)

#define LEFT_STICK_X_ID     0   //== AXIS0
#define LEFT_STICK_Y_ID     1   //== AXIS1
#define RIGHT_STICK_X_ID    2   //== AXIS2
#define RIGHT_STICK_Y_ID    3   //== AXIS3

#define LEFT_STICK_X_PIN    35
#define LEFT_STICK_Y_PIN    34
#define RIGHT_STICK_X_PIN   39
#define RIGHT_STICK_Y_PIN   36

#define ADC_PINS_LIST {LEFT_STICK_X_PIN, LEFT_STICK_Y_PIN, RIGHT_STICK_X_PIN, RIGHT_STICK_Y_PIN};

//--------------------------------------------
#define BUTTONS_COUNT 2

#define BUTTONS_CH_BASE     ADC_COUNT
#define LEFT_BUMPER_ID      0   //== BUTTON0 
#define RIGHT_BUMPER_ID     1   //== BUTTON1

#define LEFT_BUMPER_PIN     33
#define RIGHT_BUMPER_PIN    32

#define BUTTON_PINS_LIST { LEFT_BUMPER_PIN, RIGHT_BUMPER_PIN };

//temp: pin 14 output 1

//pin 22 - speaker 
//pin 25, 26 - used by I2S