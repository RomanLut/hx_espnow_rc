#pragma once

#include <Arduino.h>

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

#define MAX_MAPPING_EVENTS   128

//=====================================================================
//=====================================================================
class TXInput
{
private:
    bool trimMode;

    int lastProfileIndex = -1;

    int16_t lastChannelValue[HXRC_CHANNELS_COUNT];  
    uint32_t lastRun[MAX_MAPPING_EVENTS];
    uint16_t lastButtonsState;   //a bit for each button 1 << id
    uint16_t buttonPressEvents; // bit for each button 1 << id

    int32_t additiveAccumulator[HXRC_CHANNELS_COUNT];

    int8_t trim[3]; 

    uint32_t lastTrimTime;

    void initADCPins();
    void initButtonPins();
    void readADCEx(int pin, uint16_t* v );
    void readADC(uint32_t t);
    void readButtons(uint32_t t);

    void resetLastChannelValues();
    void resetLastRun();

    void  getChannelValuesDefault( HXChannels* channelValues );
    void  getChannelValuesMapping( HXChannels* channelValues, const JsonArray& mapping, const char* inEventName );

    int getAxisIdByName( const char* parm);
    int16_t getAxisValueByName( const char* parm, bool trim = false);

    int getButtonIdByName( const char* parm);
    int16_t getButtonValueByName( const char* parm);
    bool hasButtonPressEventByName( const char* parm );

    int16_t mapADCValue( int ADCId );
    int16_t mapButtonValue( int buttonId );

    int16_t chMul10(int16_t value, int mul);
    int16_t chClamp(int16_t value);
    int16_t getExpo( int16_t chValue, int expo );

    int16_t chAdditive(int16_t* value, int32_t* additiveAccumulator, const char* axisName, int speed, int32_t dT);
    void printADCArray( const char* title, const uint16_t* array, int shr );

    bool isValidChannelIndex(int channelIndex);

    void processAxisTrim( uint32_t t, int axisId, int trimIndex );
    void processTrim();
    void processSwitchN( HXChannels* channelValues, int channelIndex, const char* parm, int N);

    void modeEventHandler(const char* event); 
    static void staticModeEventHandler(const char* event); 

public:

    static TXInput instance;

    uint32_t lastADCRead;
    static const uint8_t ADC_PINS[ADC_COUNT];
    static const bool AXIS_INVERT[AXIS_COUNT];

    uint16_t ADC[ADC_COUNT];  //value*4 => 0...4095*4 
    uint16_t ADCMin[ADC_COUNT];  //0...4095
    uint16_t ADCMax[ADC_COUNT];  //0...4095
    uint16_t ADCMidMin[ADC_COUNT]; //0...4095
    uint16_t ADCMidMax[ADC_COUNT]; //0...4095

    uint32_t lastButtonsRead;
    static const uint8_t BUTTON_PINS[BUTTONS_COUNT];
    uint8_t buttonData[BUTTONS_COUNT];

    uint32_t lastAdditiveProcessing;

    bool calibrationDataLoadedOk;

    TXInput();

    void init();
    
    void initAxisCalibrationData();
    void loadAxisCalibrationData();
    void saveAxisCalibrationData();

    void getChannelValues( HXChannels* channelValues );

    void calibrateAxisInitADC();
    void calibrateAxisAdjustMinMaxADC();
    void finishAxisMinMaxCalibration();
    void dumpAxisMinMaxCalibration();

    void calibrateAxisInitADC2();
    void calibrateAxisAdjustMidMinMaxADC();
    void finishAxisCenterCalibration();
    void dumpAxisCenterCalibration();

    bool isButtonPressed(uint8_t buttonId);
    bool isButtonUnPressed(uint8_t buttonId);

    bool isStickMin(uint8_t stickId);
    bool isStickMiddle(uint8_t stickId);
    bool isStickMax(uint8_t stickId);

    void dumpADC();
    void dumpBatADC();

    void loop(uint32_t t);

};

