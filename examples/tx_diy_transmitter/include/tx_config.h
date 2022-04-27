#pragma once

#define WDT_TIMEOUT_SECONDS 10  

#define HC06_INTERFACE_RX_PIN  17
#define HC06_INTERFACE_TX_PIN  16

#define LED1_PIN 12
#define LED2_PIN 14
#define LED3_PIN 27
#define LED4_PIN 26

//--------------------------------------------
#define ADC_COUNT 4

#define LEFT_STICK_X_PIN 35
#define LEFT_STICK_Y_PIN 34
#define RIGHT_STICK_X_PIN 39
#define RIGHT_STICK_Y_PIN 36

#define ADC_PINS_LIST {LEFT_STICK_X_PIN, LEFT_STICK_Y_PIN, RIGHT_STICK_X_PIN, RIGHT_STICK_Y_PIN};

//--------------------------------------------
#define BUTTONS_COUNT 2

#define LEFT_BUMPER_PIN 33
#define RIGHT_BUMPER_PIN 32

#define BUTTON_PINS_LIST { LEFT_BUMPER_PIN, RIGHT_BUMPER_PIN };

//temp: pin 13 output 1