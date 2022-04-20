#include "smartport.h"
#include "smartport.h"
#include "txProfileManager.h"

//https://github.com/yaapu/FrskyTelemetryScript/wiki/FrSky-SPort-protocol-specs

//include for HXRCLOG usage
#include "HX_ESPNOW_RC_Common.h"

#define FRSKY_START_STOP  0x7e
#define FRSKY_BYTESTUFF   0x7d
#define FRSKY_STUFF_MASK  0x20

// FrSky data IDs (2 bytes)
//https://github.com/yaapu/FrskyTelemetryScript/wiki/FrSky-SPort-protocol-specs
//https://github.com/Clooney82/MavLink_FrSkySPort/wiki/1.2.-FrSky-Taranis-Telemetry
//https://github.com/opentx/opentx/blob/d67d1aa0c09d2f485c3ef7ca8c4fb1e9214a2ee7/radio/src/telemetry/frsky.h
//opentx units and precision:
//https://github.com/opentx/opentx/blob/d67d1aa0c09d2f485c3ef7ca8c4fb1e9214a2ee7/radio/src/telemetry/frsky_sport.cpp
//https://github.com/iNavFlight/inav/blob/master/src/main/telemetry/smartport.c
#define FRSKY_SPORT_ALT_ID              0x0100
#define FRSKY_SPORT_VARIO_ID            0x0110
#define FRSKY_SPORT_CURR_ID             0x0200
#define FRSKY_SPORT_VFAS_ID             0x0210  //Battery or cell voltage
#define FRSKY_SPORT_CELLS_FIRST_ID      0x0300
#define FRSKY_SPORT_CELLS_LAST_ID       0x030F
#define FRSKY_SPORT_T1_ID               0x0400  //tempterature or flight mode
#define FRSKY_SPORT_T2_ID               0x0410  //temperature or GPS state
#define FRSKY_SPORT_HOME_DIST           0x0420 
#define FRSKY_SPORT_PITCH               0x0430
#define FRSKY_SPORT_ROLL                0x0440
#define FRSKY_SPORT_FPV                 0x0450
#define FRSKY_SPORT_RPM_ID              0x0500
#define FRSKY_SPORT_RPM1                0x050E
#define FRSKY_SPORT_RPM2                0x050F
#define FRSKY_SPORT_FUEL_ID             0x0600  //battery percentage or mAh
#define FRSKY_SPORT_ACCX_ID             0x0700
#define FRSKY_SPORT_ACCY_ID             0x0710
#define FRSKY_SPORT_ACCZ_ID             0x0720
#define FRSKY_SPORT_GPS_LONG_LATI_ID    0x0800
#define FRSKY_SPORT_GPS_ALT_ID          0x0820
#define FRSKY_SPORT_GPS_SPEED_ID        0x0830
#define FRSKY_SPORT_GPS_COURS_ID        0x0840
#define FRSKY_SPORT_GPS_TIME_DATE_ID    0x0850
#define FRSKY_SPORT_ASPD                0x0A00 //pitot sensor speed
#define FRSKY_SPORT_A3                  0x0900 
#define FRSKY_SPORT_A4                  0x0910 //average battery voltage
#define FRSKY_SPORT_DIY_FIRST_ID        0x5000
#define FRSKY_SPORT_DIY_LAST_ID         0x52ff
//0x5000 status text
//0x5001 AP status
//0x5002 GPS Status
//0x5003 Battery 1 status
//0x5004 Home
//0x5005 Vel and Yaw
//0x5007 parameters
//0x5008 Battery 2 status
//0x500A rpm sensors 1 and 2
//0x500B terrain data
//0x500C terrain data

#define FRSKY_SPORT_DIY_NOISE_FLOOR_ID      0x5250
#define FRSKY_SPORT_DIY_SNR_ID              0x5251

#define FRSKY_SPORT_DIY_RX_RSSI_ID          0x5252
#define FRSKY_SPORT_DIY_RX_RSSI_DBM_ID      0x5253
#define FRSKY_SPORT_DIY_RX_NOISE_FLOOR_ID   0x5254
#define FRSKY_SPORT_DIY_RX_SNR_ID           0x5255

#define FRSKY_SPORT_DIY_PROFILE_ID          0x5256

#define FRSKY_SPORT_DIY_DEBUG_1_ID          0x5260
#define FRSKY_SPORT_DIY_DEBUG_2_ID          0x5261
#define FRSKY_SPORT_DIY_DEBUG_3_ID          0x5262

#define FRSKY_SPORT_VALID_FRAME_RATE_ID 0xF010  //UNIT_PERCENT displayed as 100-data in OpenTX
#define FRSKY_SPORT_RSSI_ID             0xf101  //low byte - rssi in dbm, 0x64 = 100dbm
#define FRSKY_SPORT_ADC1_ID             0xf102 // A1  Volts Ratio 13.2 Precision 1
#define FRSKY_SPORT_ADC2_ID             0xf103 // A2  Volts Ratio 13.2 Precision 1
#define FRSKY_SPORT_BATT_ID             0xf104 //Volts, Ratio 13.2 Precision 2 
#define FRSKY_SPORT_SWR_ID              0xf105
#define FRSKY_SPORT_R9_PWR_ID           0xF107 //sent in dbm, displayed in mW, dbm=>mw = {{0, 1}, {5, 3}, {10, 10}, {13, 20}, {14, 25}, {20, 100}, {23, 200}, {27, 500}, {30, 1000}};
#define FRSKY_SPORT_SP2UART_A_ID        0xFD00
#define FRSKY_SPORT_SP2UART_B_ID        0xFD01

#define FRSKY_SPORT_RX_LQI_ID           0xFFFC
#define FRSKY_SPORT_TX_LQI_ID           0xFFFD
#define FRSKY_SPORT_TX_RSSI_ID          0xFFFE

// FrSky sensor IDs (this also happens to be the order in which they're broadcast from an X8R)
#define FRSKY_SPORT_DEVICE_1    0xa1
#define FRSKY_SPORT_DEVICE_2    0x22
#define FRSKY_SPORT_DEVICE_3    0x83
#define FRSKY_SPORT_DEVICE_4    0xe4
#define FRSKY_SPORT_DEVICE_5    0x45
#define FRSKY_SPORT_DEVICE_6    0xc6
#define FRSKY_SPORT_DEVICE_7    0x67
#define FRSKY_SPORT_DEVICE_8    0x48
#define FRSKY_SPORT_DEVICE_9    0xe9
#define FRSKY_SPORT_DEVICE_10   0x6a
#define FRSKY_SPORT_DEVICE_11   0xcb
#define FRSKY_SPORT_DEVICE_12   0xac
#define FRSKY_SPORT_DEVICE_13   0xd
#define FRSKY_SPORT_DEVICE_14   0x8e
#define FRSKY_SPORT_DEVICE_15   0x2f
#define FRSKY_SPORT_DEVICE_16   0xd0
#define FRSKY_SPORT_DEVICE_17   0x71
#define FRSKY_SPORT_DEVICE_18   0xf2
#define FRSKY_SPORT_DEVICE_19   0x53
#define FRSKY_SPORT_DEVICE_20   0x34
#define FRSKY_SPORT_DEVICE_21   0x95
#define FRSKY_SPORT_DEVICE_22   0x16
#define FRSKY_SPORT_DEVICE_23   0xb7
#define FRSKY_SPORT_DEVICE_24   0x98
#define FRSKY_SPORT_DEVICE_25   0x39
#define FRSKY_SPORT_DEVICE_26   0xba
#define FRSKY_SPORT_DEVICE_27   0x1b

#define FRSKY_SPORT_A2_MAX 124    // A2 voltage is represented by a value in the range 0-255. A value of 16 results in 1.6V, 124 is 12.4V, etc

//=====================================================================
//=====================================================================
Smartport::Smartport() 
{
    this->setValues = 0;
}

//=====================================================================
//=====================================================================
void Smartport::init( HardwareSerial* serial, int tx_pin )
{
    this->serial = serial;
    this->serial->begin(57600, SERIAL_8N1, -1, tx_pin, true );  
    pinMode(tx_pin, OUTPUT);  //call after serial->begin()
	this->lastSend = millis();
    this->lastSensor = 0;
}

//=====================================================================
//=====================================================================
void Smartport::sendByte(uint8_t byte)
{
	// CRC update
	this->crc += byte;//0-1FF
	this->crc += this->crc >> 8;//0-100
	this->crc &= 0x00ff;
	this->crc += this->crc >> 8;//0-0FF
	this->crc &= 0x00ff;

	if ( (byte == FRSKY_START_STOP) || (byte == FRSKY_BYTESTUFF) ) 
    {
		this->serial->write(FRSKY_BYTESTUFF);
		byte &= ~FRSKY_STUFF_MASK;
	}

	this->serial->write(byte);
}

//=====================================================================
//=====================================================================
void Smartport::sendCrc()
{
	this->sendByte(0xFF - this->crc);
}

//=====================================================================
//=====================================================================
void Smartport::sendValue(uint16_t id, uint32_t value)
{
	this->crc = 0; // Reset CRC
	this->sendByte(0x10);// DATA_FRAME
	uint8_t *bytes = (uint8_t*)&id;
	this->sendByte(bytes[0]);
	this->sendByte(bytes[1]);
	bytes = (uint8_t*)&value;
	this->sendByte(bytes[0]);
	this->sendByte(bytes[1]);
	this->sendByte(bytes[2]);
	this->sendByte(bytes[3]);
	this->sendCrc();
}

//=====================================================================
//=====================================================================
void Smartport::sendDeviceValue(uint8_t deviceId, uint16_t sensorId, uint32_t value)
{
    this->serial->write( FRSKY_START_STOP ); //0x7e
    this->serial->write( deviceId); //0x98

    //low byte - rssi in dbm, 0x64 = 100dbm
    this->sendValue(sensorId, value);
}

//=====================================================================
//=====================================================================
bool Smartport::findNextValue()
{
    uint8_t count = SVI_COUNT;
    while ( true )
    {
        this->lastSensor++;
        if ( this->lastSensor == SVI_COUNT ) this->lastSensor = 0;

        uint32_t sensorMask = (1<<this->lastSensor);
        if ( this->setValues & sensorMask ) 
        {
            this->setValues &= ~sensorMask;
            break;
        }

        count--;
        if ( count == 0 ) return false;
    }
    return true;
}

//=====================================================================
//=====================================================================
void Smartport::loop()
{
    unsigned long t = millis();
    unsigned long deltaT = t - this->lastSend;

    if ( deltaT > 11 )
    {
        this->lastSend = t;

        if ( !this->findNextValue() ) return;

        switch (this->lastSensor)
        {
        case SVI_RSSI:
	        this->sendDeviceValue(FRSKY_SPORT_DEVICE_24, FRSKY_SPORT_RSSI_ID, this->values[this->lastSensor]);
            break;

        case SVI_RSSI_DBM:
	        this->sendDeviceValue(FRSKY_SPORT_DEVICE_24, FRSKY_SPORT_TX_RSSI_ID, this->values[this->lastSensor]);
            break;

        case SVI_NOISE_FLOOR:
	        this->sendDeviceValue(FRSKY_SPORT_DEVICE_24, FRSKY_SPORT_DIY_NOISE_FLOOR_ID, this->values[this->lastSensor]);
            break;

        case SVI_SNR:
	        this->sendDeviceValue(FRSKY_SPORT_DEVICE_24, FRSKY_SPORT_DIY_SNR_ID, this->values[this->lastSensor]);
            break;

        case SVI_RX_RSSI:
	        this->sendDeviceValue(FRSKY_SPORT_DEVICE_24, FRSKY_SPORT_DIY_RX_RSSI_ID, this->values[this->lastSensor]);
            break;

        case SVI_RX_RSSI_DBM:
	        this->sendDeviceValue(FRSKY_SPORT_DEVICE_24, FRSKY_SPORT_DIY_RX_RSSI_DBM_ID, this->values[this->lastSensor]);
            break;

        case SVI_RX_NOISE_FLOOR:
	        this->sendDeviceValue(FRSKY_SPORT_DEVICE_24, FRSKY_SPORT_DIY_RX_NOISE_FLOOR_ID, this->values[this->lastSensor]);
            break;

        case SVI_RX_SNR:
	        this->sendDeviceValue(FRSKY_SPORT_DEVICE_24, FRSKY_SPORT_DIY_RX_SNR_ID, this->values[this->lastSensor]);
            break;

        case SVI_R9_PWR_ID:
	        this->sendDeviceValue(FRSKY_SPORT_DEVICE_24, FRSKY_SPORT_R9_PWR_ID, this->values[this->lastSensor]);
            break;

        case SVI_VALID_FRAME_RATE:
	        this->sendDeviceValue(FRSKY_SPORT_DEVICE_24, FRSKY_SPORT_VALID_FRAME_RATE_ID, this->values[this->lastSensor]);
            break;

        case SVI_PROFILE_ID:
	        this->sendDeviceValue(FRSKY_SPORT_DEVICE_24, FRSKY_SPORT_DIY_PROFILE_ID, this->values[this->lastSensor]);
            break;

        case SVI_DEBUG_1:
	        this->sendDeviceValue(FRSKY_SPORT_DEVICE_24, FRSKY_SPORT_DIY_DEBUG_1_ID, this->values[this->lastSensor]);
            break;

        case SVI_DEBUG_2:
	        this->sendDeviceValue(FRSKY_SPORT_DEVICE_24, FRSKY_SPORT_DIY_DEBUG_2_ID, this->values[this->lastSensor]);
            break;

        case SVI_DEBUG_3:
	        this->sendDeviceValue(FRSKY_SPORT_DEVICE_24, FRSKY_SPORT_DIY_DEBUG_3_ID, this->values[this->lastSensor]);
            break;

        case SVI_A1:
	        this->sendDeviceValue(FRSKY_SPORT_DEVICE_4, FRSKY_SPORT_ADC1_ID, this->values[this->lastSensor]);
            break;

        case SVI_A2:
	        this->sendDeviceValue(FRSKY_SPORT_DEVICE_4, FRSKY_SPORT_ADC2_ID, this->values[this->lastSensor]);
            break;

        case SVI_VFAS:
	        this->sendDeviceValue(FRSKY_SPORT_DEVICE_4, FRSKY_SPORT_VFAS_ID, this->values[this->lastSensor]);
            break;

        case SVI_ALTITUDE:
	        this->sendDeviceValue(FRSKY_SPORT_DEVICE_11, FRSKY_SPORT_ALT_ID, this->values[this->lastSensor]);
            break;

        }
    }
}




/*

this->serial->write( 0x7e );
this->serial->write( 0x98 );   
this->serial->write( 0x10 );   
this->serial->write( 0x01 );   
this->serial->write( 0xf1 );   
this->serial->write( 0x64 );   
this->serial->write( 0xcd );   
this->serial->write( 0x00 );   
this->serial->write( 0x5e );   
this->serial->write( 0x6c );   
*/


/*
        case 3:

            this->serial->write( FRSKY_START_STOP ); //0x7e
            this->serial->write( FRSKY_SPORT_DEVICE_10); 

            this->sendDIYValue(FRSKY_SPORT_ACCX_ID, 0);
            break;

        case 4:

            this->serial->write( FRSKY_START_STOP ); //0x7e
            this->serial->write( FRSKY_SPORT_DEVICE_10); 

            this->sendDIYValue(FRSKY_SPORT_ACCY_ID, 1);
            break;

        case 5:

            this->serial->write( FRSKY_START_STOP ); //0x7e
            this->serial->write( FRSKY_SPORT_DEVICE_10); 

            this->sendDIYValue(FRSKY_SPORT_ACCZ_ID, 2);
            break;
*/
/*
            case FRSKY_SPORT_DEVICE_8:
            FrSkySport_sendACCX();
            break;
            case FRSKY_SPORT_DEVICE_9:
            FrSkySport_sendACCY();
            break;
            case FRSKY_SPORT_DEVICE_10:
            FrSkySport_sendACCZ();
            break;
            case FRSKY_SPORT_DEVICE_11:
            FrSkySport_sendAltitude();
            break;
            case FRSKY_SPORT_DEVICE_12:
            FrSkySport_sendAltVario();
            break;
            case FRSKY_SPORT_DEVICE_13:
	        this->sendValue(FRSKY_SPORT_GPS_COURS_ID, (uint32_t)(10 + 360) % 360 * 100); // 1 deg = 100, 0 - 359000
            break;
            case FRSKY_SPORT_DEVICE_14:
            FrSkySport_sendGPSSpeed();
            break;
            case FRSKY_SPORT_DEVICE_15:
            FrSkySport_sendGPSAltitude();
            break;
            case FRSKY_SPORT_DEVICE_16:
            FrSkySport_sendGPSCoordinate();
            break;
*/
