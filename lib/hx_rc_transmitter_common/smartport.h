#pragma once

#include <Arduino.h>

#define SVI_RSSI                0
#define SVI_RSSI_DBM            1
#define SVI_NOISE_FLOOR         2
#define SVI_SNR                 3
#define SVI_RX_RSSI             4
#define SVI_RX_RSSI_DBM         5
#define SVI_RX_NOISE_FLOOR      6
#define SVI_RX_SNR              7
#define SVI_VALID_FRAME_RATE    8 
#define SVI_R9_PWR_ID           9
#define SVI_PROFILE_ID          10  
#define SVI_A1                  11  
#define SVI_A2                  12
#define SVI_VFAS                13
#define SVI_ALTITUDE            14
#define SVI_DEBUG_1             15
#define SVI_DEBUG_2             16
#define SVI_DEBUG_3             17  
#define SVI_COUNT               18

//=====================================================================
//=====================================================================
//Frsky Smart Port protocol implementation
//Usage: 
//1) Instantiate class
//2) periodically set sensor values using setXXX() methods, at any frequency. 
//   You have to update each sensor at leat once per second, otherwise  OpenTX will fire lost telemetry alarm.
//3) call loop() with at least 100 Hz frequency
//4) class will write updated sensors to serial stream.
class Smartport
{
private:
    unsigned long lastSend;
    uint8_t lastSensor;

    uint32_t setValues;
    uint32_t values[SVI_COUNT];

    unsigned short crc;

    HardwareSerial* serial;

    void sendByte(uint8_t byte);
    void sendCrc();
    void sendValue(uint16_t id, uint32_t value);
    void sendDeviceValue(uint8_t deviceId, uint16_t sensorId, uint32_t value);
    bool findNextValue();

public:

    Smartport();

    void init( HardwareSerial* serial, int tx_pin );
    void loop();

    void setSportValue(uint8_t id, uint32_t value)
    {
        this->values[id] = value;
        this->setValues |= 1<<id;
    }
    
    //Opentx: RSSI, Db, precision 1
    //100=>100
    void setRSSI( uint8_t value)
    {
        this->setSportValue(SVI_RSSI, value);
    } 

    //Sensor: TRSS, Db, precision 1
    //100=>100
    void setRSSIDbm( uint8_t value)
    {
        this->setSportValue(SVI_RSSI_DBM, value);
    } 

    //Sensor: 5250, precision 1
    //100=>100
    void setNoiseFloor( uint8_t value)
    {
        this->setSportValue(SVI_NOISE_FLOOR, value);
    } 

    //Sensor: 5251, precision 1
    //100=>100
    void setSNR( uint8_t value)
    {
        this->setSportValue(SVI_SNR, value);
    } 

    //Opentx: TRSS, Db, precision 2 
    //100=>100
    void setRXRSSI( uint8_t value)
    {
        this->setSportValue(SVI_RX_RSSI, value);
    } 

    //Sensor: 5253
    //100=>100
    void setRXRSSIDbm( uint8_t value)
    {
        this->setSportValue(SVI_RX_RSSI_DBM, value);
    } 

    //Sensor: 5254, precision 1
    //100=>100
    void setRXNoiseFloor( uint8_t value)
    {
        this->setSportValue(SVI_RX_NOISE_FLOOR, value);
    } 

    //Sensor: 5255, precision 1
    //100=>100
    void setRXSNR( uint8_t value)
    {
        this->setSportValue(SVI_RX_SNR, value);
    } 

    //R9PW 
    //sent in dbm, displayed in mW, dbm=>mw = {{0, 1}, {5, 3}, {10, 10}, {13, 20}, {14, 25}, {20, 100}, {23, 200}, {27, 500}, {30, 1000}};
    //20dbm=>100mW
    void setR9PWR( uint8_t value)
    {
        this->setSportValue(SVI_R9_PWR_ID, value);
    } 

    //VFR, %, displayed as 100-% in OpenTX
    //21=>79
    void setValidPacketRate( uint8_t value)
    {
        this->setSportValue(SVI_VALID_FRAME_RATE, value);
    } 

    //Sensor: 5256, precision 1
    //100=>100
    void setProfileId( uint8_t value)
    {
        this->setSportValue(SVI_PROFILE_ID, value);
    } 

    //OpenTX: Volts, precision 1, Ratio 13.2
    //100=>100/255*13.2=5.2V
    //If ratio is disabled: 100=>10
    void setA1( uint32_t value)
    {
        this->setSportValue(SVI_A1, value);
    }   

    //OpenTX: Volts, precision 1, Ratio 13.2
    //100=>100/255*13.2 = 5.2V
    //If ratio is disabled: 100=>10
    void setA2( uint32_t value)
    {
        this->setSportValue(SVI_A2, value);
    }
       
    //Battery voltage, Opentx: Volts, precision 2
    //100=>1V
    void setVFAS( uint16_t value)
    {
        this->setSportValue(SVI_VFAS, value);
    }  

    //Baro Altitude, meters, precision 2, Auto offset
    //100=>1m
    void setAltitude( uint32_t value)
    {
        this->setSportValue(SVI_ALTITUDE, value);
    }  

    //5260
    void setDebug1( uint32_t value)
    {
        this->setSportValue(SVI_DEBUG_1, value);
    }  

    //5261
    void setDebug2( uint32_t value)
    {
        this->setSportValue(SVI_DEBUG_2, value);
    }  

    //5262
    void setDebug3( uint32_t value)
    {
        this->setSportValue(SVI_DEBUG_3, value);
    }  

};

