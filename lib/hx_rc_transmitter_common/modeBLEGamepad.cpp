#include <Arduino.h>
#include <ArduinoOTA.h>
#include <esp_task_wdt.h>

//include for HXRCLOG usage
#include "HX_ESPNOW_RC_Common.h"

#include "modeBLEGamepad.h"
#include "txProfileManager.h"

#include <BleGamepad.h>		// https://github.com/lemmingDev/ESP32-BLE-Gamepad

//vid,pid:
//https://githublab.com/repository/issues/lemmingDev/ESP32-BLE-Gamepad/87
//https://developer.x-plane.com/article/creating-joystick-configuration-joy-files/
BleGamepad bleGamepad("HXRCBLE","Roman Lut");

ModeBLEGamepad ModeBLEGamepad::instance;
const char* ModeBLEGamepad::name = "BLEGAMEPAD";

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

    this->connected = false;
}

//=====================================================================
//=====================================================================
void ModeBLEGamepad::sendControls(const HXChannels* channels)
{
    int16_t v = channels->channelValue[0];
    if ( axes[0]!=v)
    {
        axes[0]=v;
        bleGamepad.setX( map(v, 1000,2000, -32767, 32767));
    }

    v = channels->channelValue[1];
    if ( axes[1]!=v)
    {
        axes[1]=v;
        bleGamepad.setY( map(v, 1000,2000, -32767, 32767));
    }

    v = channels->channelValue[2];
    if ( axes[2]!=v)
    {
        axes[2]=v;
        bleGamepad.setZ( map(v, 1000,2000, -32767, 32767));
    }

    v = channels->channelValue[3];
    if ( axes[3]!=v)
    {
        axes[3]=v;
        bleGamepad.setRX( map(v, 1000,2000, -32767, 32767));
    }

    v = channels->channelValue[4];
    if ( axes[4]!=v)
    {
        axes[4]=v;
        bleGamepad.setRY( map(v, 1000,2000, -32767, 32767));
    }

    v = channels->channelValue[5];
    if ( axes[5]!=v)
    {
        axes[5]=v;
        bleGamepad.setRZ( map(v, 1000,2000, -32767, 32767));
    }

    for ( int i = 0; i < BLE_GAMEPAD_BUTTON_COUNT; i++ )
    {
        bool v = channels->channelValue[BLE_GAMEPAD_AXIS_COUNT + i] > 1750;
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
void ModeBLEGamepad::start( JsonDocument* json )
{
    ModeBase::start( json );
    
    HXRCLOG.println("BLE Gamepad start\n");      

    pinMode(LED_PIN,OUTPUT);
    digitalWrite(LED_PIN, HIGH );

    esp_task_wdt_reset();

    bleGamepad.begin(BLE_GAMEPAD_BUTTON_COUNT);
}

//=====================================================================
//=====================================================================
void ModeBLEGamepad::loop(
        const HXChannels* channels,
        HC06Interface* externalBTSerial,
        Smartport* sport
    )
{
    ModeBase::loop(channels, externalBTSerial, sport);

    if (channels->isFailsafe) 
    {
        digitalWrite(LED_PIN, LOW );
    }
    else
    {
        digitalWrite(LED_PIN, ((millis() & 512) > 0 ) ? HIGH : LOW );
    }

    if ( sport != NULL )
    {
        sport->setRSSI( bleGamepad.isConnected() ? 100 : 0);
        sport->loop();
    }

    if(bleGamepad.isConnected()) 
    {
        this->sendControls(channels);

        if ( !this->connected )
        {
            this->fire(EVENT_CONNECTED);
            this->connected = true;
        }
    }
    else
    {
        if ( this->connected )
        {
            this->fire(EVENT_DISCONNECTED);
            this->connected = false;
        }
    }

    if ( haveToChangeProfile() )
    {
        rebootToRequestedProfile();
    }
}
