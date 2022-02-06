#include <Arduino.h>
#include <ArduinoOTA.h>
#include <esp_task_wdt.h>

//include for HXRCLOG usage
#include "HX_ESPNOW_RC_Common.h"

#include "modeBLEGamepad.h"
#include "tx_config.h"

#include <BleGamepad.h>		// https://github.com/lemmingDev/ESP32-BLE-Gamepad

BleGamepad bleGamepad("HXRCBLE");

ModeBLEGamepad ModeBLEGamepad::instance;

//=====================================================================
//=====================================================================
ModeBLEGamepad::ModeBLEGamepad() 
{
    for ( int i = 0; i < BLE_GAMEPAD_AXIS_COUNT; i++ )
    {
        axes[i] = 1500;
    }

    for ( int i = 0; i < BLE_GAMEPAD_BUTTON_COUNT; i++ )
    {
        buttons[i] = false;
    }
}

//=====================================================================
//=====================================================================
void ModeBLEGamepad::sendControls(HXSBUSDecoder* sbusDecoder)
{
    int16_t v = sbusDecoder->getChannelValueInRange(0, 1000, 2000);
    if ( axes[0]!=v)
    {
        axes[0]=v;
        bleGamepad.setX( map(v, 1000,2000, -32767, 32767));
    }

    v = sbusDecoder->getChannelValueInRange(1, 1000, 2000);
    if ( axes[1]!=v)
    {
        axes[1]=v;
        bleGamepad.setY( map(v, 1000,2000, -32767, 32767));
    }

    v = sbusDecoder->getChannelValueInRange(2, 1000, 2000);
    if ( axes[2]!=v)
    {
        axes[2]=v;
        bleGamepad.setZ( map(v, 1000,2000, -32767, 32767));
    }

    v = sbusDecoder->getChannelValueInRange(3, 1000, 2000);
    if ( axes[3]!=v)
    {
        axes[3]=v;
        bleGamepad.setRX( map(v, 1000,2000, -32767, 32767));
    }

    v = sbusDecoder->getChannelValueInRange(4, 1000, 2000);
    if ( axes[4]!=v)
    {
        axes[4]=v;
        bleGamepad.setRY( map(v, 1000,2000, -32767, 32767));
    }

    v = sbusDecoder->getChannelValueInRange(5, 1000, 2000);
    if ( axes[5]!=v)
    {
        axes[5]=v;
        bleGamepad.setRZ( map(v, 1000,2000, -32767, 32767));
    }

    for ( int i = 0; i < BLE_GAMEPAD_BUTTON_COUNT; i++ )
    {
        bool v = sbusDecoder->getChannelValueInRange(BLE_GAMEPAD_AXIS_COUNT + i, 1000, 2000) > 1750;
        if ( buttons[i] != v )
        {
            if ( v) 
            {
                bleGamepad.press( BUTTON_1 + i);
            }
            else
            {
                bleGamepad.release( BUTTON_1 + i);
            }
            buttons[i] = v;
        }
    }
}

//=====================================================================
//=====================================================================
void ModeBLEGamepad::start()
{
    ModeBase::start();
    
    HXRCLOG.println("BLE Gamepad start\n");      

    pinMode(LED_PIN,OUTPUT);
    digitalWrite(LED_PIN, HIGH );

    esp_task_wdt_reset();

    bleGamepad.begin(BLE_GAMEPAD_BUTTON_COUNT);
}

//=====================================================================
//=====================================================================
void ModeBLEGamepad::loop(
        HXSBUSDecoder* sbusDecoder,
        HC06Interface* externalBTSerial,
        Smartport* sport
    )
{
    ModeBase::loop(sbusDecoder, externalBTSerial, sport);

    if (sbusDecoder->isFailsafe() ) 
    {
        digitalWrite(LED_PIN, LOW );
    }
    else
    {
        digitalWrite(LED_PIN, ((millis() & 512) > 0 ) ? HIGH : LOW );
    }

#ifdef USE_SPORT  
    sport->setRSSI( bleGamepad.isConnected() ? 100 : 0);
    sport->loop();
#endif

    if(bleGamepad.isConnected()) 
    {
        this->sendControls(sbusDecoder);
    }


    if ( haveToChangeProfile() )
    {
        rebootToRequestedProfile();
    }
}
