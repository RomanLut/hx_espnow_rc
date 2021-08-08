#include "Smartport.h"
#include "tx_config.h"

//https://github.com/yaapu/FrskyTelemetryScript/wiki/FrSky-SPort-protocol-specs

//include for HXRCLOG usage
#include "HX_ESPNOW_RC_COMMON.h"

#define FRSKY_START_STOP  0x7e
#define FRSKY_BYTESTUFF   0x7d
#define FRSKY_STUFF_MASK  0x20

// FrSky data IDs (2 bytes)
#define FRSKY_SPORT_RSSI_ID           0xf101
#define FRSKY_SPORT_ADC1_ID           0xf102 // A1
#define FRSKY_SPORT_ADC2_ID           0xf103 // A2
#define FRSKY_SPORT_BATT_ID           0xf104
#define FRSKY_SPORT_SWR_ID            0xf105
#define FRSKY_SPORT_T1_ID             0x0400
#define FRSKY_SPORT_T2_ID             0x0410
#define FRSKY_SPORT_RPM_ID            0x0500
#define FRSKY_SPORT_FUEL_ID           0x0600
#define FRSKY_SPORT_ALT_ID            0x0100
#define FRSKY_SPORT_VARIO_ID          0x0110
#define FRSKY_SPORT_ACCX_ID           0x0700
#define FRSKY_SPORT_ACCY_ID           0x0710
#define FRSKY_SPORT_ACCZ_ID           0x0720
#define FRSKY_SPORT_CURR_ID           0x0200
#define FRSKY_SPORT_VFAS_ID           0x0210
#define FRSKY_SPORT_CELLS_ID          0x0300
#define FRSKY_SPORT_GPS_LONG_LATI_ID  0x0800
#define FRSKY_SPORT_GPS_ALT_ID        0x0820
#define FRSKY_SPORT_GPS_SPEED_ID      0x0830
#define FRSKY_SPORT_GPS_COURS_ID      0x0840
#define FRSKY_SPORT_GPS_TIME_DATE_ID  0x0850

// FrSky sensor IDs (this also happens to be the order in which they're broadcast from an X8R)
// NOTE: As FrSky puts out more sensors let's try to add comments here indicating which is which
#define FRSKY_SPORT_DEVICE_1   0xa1
#define FRSKY_SPORT_DEVICE_2   0x22
#define FRSKY_SPORT_DEVICE_3   0x83
#define FRSKY_SPORT_DEVICE_4   0xe4
#define FRSKY_SPORT_DEVICE_5   0x45
#define FRSKY_SPORT_DEVICE_6   0xc6
#define FRSKY_SPORT_DEVICE_7   0x67
#define FRSKY_SPORT_DEVICE_8   0x48
#define FRSKY_SPORT_DEVICE_9   0xe9
#define FRSKY_SPORT_DEVICE_10  0x6a
#define FRSKY_SPORT_DEVICE_11  0xcb
#define FRSKY_SPORT_DEVICE_12  0xac
#define FRSKY_SPORT_DEVICE_13  0xd
#define FRSKY_SPORT_DEVICE_14  0x8e
#define FRSKY_SPORT_DEVICE_15  0x2f
#define FRSKY_SPORT_DEVICE_16  0xd0
#define FRSKY_SPORT_DEVICE_17  0x71
#define FRSKY_SPORT_DEVICE_18  0xf2
#define FRSKY_SPORT_DEVICE_19  0x53
#define FRSKY_SPORT_DEVICE_20  0x34
#define FRSKY_SPORT_DEVICE_21  0x95
#define FRSKY_SPORT_DEVICE_22  0x16
#define FRSKY_SPORT_DEVICE_23  0xb7
#define FRSKY_SPORT_DEVICE_24  0x98
#define FRSKY_SPORT_DEVICE_25  0x39
#define FRSKY_SPORT_DEVICE_26  0xba
#define FRSKY_SPORT_DEVICE_27  0x1b

#define FRSKY_SPORT_A2_MAX 124    // A2 voltage is represented by a value in the range 0-255. A value of 16 results in 1.6V, 124 is 12.4V, etc

//=====================================================================
//=====================================================================
Smartport::Smartport(HardwareSerial* serial) 
{
    this->serial = serial;
    this->RSSI = 0;
    this->A1 = 0;
    this->A2 = 0;
}

//=====================================================================
//=====================================================================
void Smartport::init()
{
    this->serial->begin(57600, SERIAL_8N1, -1, SPORT_PIN, true );  
    pinMode(SPORT_PIN, OUTPUT);  //call after serial->begin()
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
void Smartport::sendRSSI()
{
    //low byte - rssi in dbm, 0x64 = 100dbm
	uint32_t opentx_val = this->RSSI;
	this->sendValue(FRSKY_SPORT_RSSI_ID, (opentx_val));
}

//=====================================================================
//=====================================================================
void Smartport::sendA1Voltage()
{
	uint32_t opentx_val = this->A1;
	this->sendValue(FRSKY_SPORT_ADC1_ID, (opentx_val));
}

//=====================================================================
//=====================================================================
void Smartport::sendA2Value()
{
	uint32_t opentx_val = this->A2;
	this->sendValue(FRSKY_SPORT_ADC2_ID, (opentx_val));
}


//=====================================================================
//=====================================================================
void Smartport::sendHeading()
{
	uint32_t otx_heading = (uint32_t)(10 + 360) % 360 * 100;
	this->sendValue(FRSKY_SPORT_GPS_COURS_ID, otx_heading); // 1 deg = 100, 0 - 359000
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

        switch (this->lastSensor)
        {
        case 0:
            this->serial->write( FRSKY_START_STOP ); //0x7e
            this->serial->write( FRSKY_SPORT_DEVICE_24); //0x98

            this->sendRSSI();

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
            break;

        case 1:

            this->serial->write( FRSKY_START_STOP ); //0x7e
            this->serial->write( FRSKY_SPORT_DEVICE_4); //0xe4

            this->sendA1Voltage();
            break;

        case 2:

            this->serial->write( FRSKY_START_STOP ); //0x7e
            this->serial->write( FRSKY_SPORT_DEVICE_4); //0xe4

            this->sendA2Value();
            break;

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
            FrSkySport_sendHeading();
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
        }

        this->lastSensor++;
        if ( this->lastSensor == 3 )
        {
            this->lastSensor = 0;
        }
    }
}

//=====================================================================
//=====================================================================
void Smartport::setRSSI(uint8_t value)
{
    this->RSSI = value;
}

//=====================================================================
//=====================================================================
void Smartport::setA1(uint32_t value)
{
    this->A1 = value;
}

//=====================================================================
//=====================================================================
void Smartport::setA2(uint32_t value)
{
    this->A2 = value;
}



