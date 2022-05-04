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
    int lastProfileIndex = -1;

    int16_t lastChannelValue[HXRC_CHANNELS_COUNT];  
    uint16_t lastButtonsState;   //a bit for each button 1 << id
    uint16_t buttonPressEvents; // bit for each button 1 << id

    void initAxisPins();
    void initButtonPins();
    void readADCEx(int pin, uint16_t* v );
    void readADC(uint32_t t);
    void readButtons(uint32_t t);

    void resetLastChannelValues();

    void  getChannelValuesDefault( out HXChannels* channelValues );
    void  getChannelValuesMapping( inout HXChannels* channelValues, const JsonArray& mapping, bool startup );

    int getAxisIdByName( const char* parm);
    int16_t getAxisValueByName( const char* parm);

    int getButtonIdByName( const char* parm);
    int16_t getButtonValueByName( const char* parm);
    bool hasButtonPressEventByName( const char* parm );

    int16_t mapADCValue( int ADCId );
    int16_t mapButtonValue( int buttonId );

    int16_t chMul10(int16_t value, int mul);
    int16_t chClamp(int16_t value);

    int16_t chAdditive(int16_t value, const char* axisName, int speed, int32_t dT);

public:

    static TXInput instance;

    uint32_t lastADCRead;
    static const uint8_t ADC_PINS[ADC_COUNT];
    uint16_t ADC[ADC_COUNT];
    uint16_t ADCMin[ADC_COUNT];
    uint16_t ADCMax[ADC_COUNT];
    uint16_t ADCMidMin[ADC_COUNT];
    uint16_t ADCMidMax[ADC_COUNT];

    uint32_t lastButtonsRead;
    static const uint8_t BUTTON_PINS[BUTTONS_COUNT];
    uint8_t buttonData[BUTTONS_COUNT];

    uint32_t lastAdditiveProcessing;

    TXInput();

    void init();
    
    void initCalibrationData();
    void loadCalibrationData();
    void saveCalibrationData();

    void getChannelValues( inout HXChannels* channelValues );

    bool isCalibrateGesture();
    bool isInitGesture();

    void calibrateInitADC();
    void calibrateAdjustMinMaxADC();
    bool isMinMaxCalibrationSuccessfull();

    void calibrateInitADC2();
    void calibrateAdjustMidMinMaxADC();
    bool isCenterCalibrationSuccessfull();

    bool isButtonPressed(uint8_t buttonId);
    bool isButtonUnPressed(uint8_t buttonId);

    bool isStickMin(uint8_t stickId);
    bool isStickMiddle(uint8_t stickId);
    bool isStickMax(uint8_t stickId);

    void loop(uint32_t t);

};

