#pragma once

#include <Arduino.h>
#include <Audio.h>

#include <stdio.h>
#include <stdarg.h>

#include <esp_task_wdt.h>

#include <ArduinoJson.h>

#include "HX_ESPNOW_RC_Master.h"
#include "HX_ESPNOW_RC_SerialBuffer.h"

#include "txProfileManager.h"

#include "tx_config.h"
#include "HC06Interface.h"

#include "modeIdle.h"

#include "hx_channels.h"

#include <SPIFFS.h> 


//=====================================================================
//=====================================================================
class TXInput
{
private:
    void initAxisPins();
    void initButtonPins();
    void readADCEx(int pin, uint16_t* v );
    void readADC(uint32_t t);
    void readButtons(uint32_t t);

public:

    static TXInput instance;

    uint32_t lastADCRead;
    static const uint8_t ADC_PINS[ADC_COUNT];
    uint16_t ADC[ADC_COUNT];
    uint16_t ADCMin[ADC_COUNT];
    uint16_t ADCMax[ADC_COUNT];
    uint16_t ADCMidMin[ADC_COUNT];
    uint16_t ADCMidMax[ADC_COUNT];

    uint32_t lastButtonsRead = millis();
    static const uint8_t BUTTON_PINS[BUTTONS_COUNT];
    uint8_t buttonData[BUTTONS_COUNT];

    TXInput();

    void init();
    
    void initCalibrationData();
    void loadCalibrationData();
    void saveCalibrationData();

    void  getChannelValues( HXChannels* channelValues );

    bool isCalibrateGesture();
    bool isInitGesture();

    void calibrateInitADC();
    void calibrateAdjustMinMaxADC();
    bool isMinMaxCalibrationSuccessfull();

    void calibrateInitADC2();
    void calibrateAdjustMidMinMaxADC();
    bool isCenterCalibrationSuccessfull();

    void loop(uint32_t t);

};

