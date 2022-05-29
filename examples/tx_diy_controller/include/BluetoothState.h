#pragma once

#include <Arduino.h>

#include <stdio.h>
#include <stdarg.h>

#include "Debounce.h"
#include "tx_config.h"
#include "AudioManager.h"

#define BLUETOOTH_CHECK_PERIOD_FIRST_MS 6000
#define BLUETOOTH_CHECK_PERIOD_MS 3000

//=====================================================================
//=====================================================================
class BluetoothState: public Debounce
{
protected:    

    bool readState( bool lastState ) override
    {
        return digitalRead(HC06_LED_PIN) == HIGH;        
    }

    void onDebouncedChange( bool state ) override
    {
      AudioManager::instance.play( state ? "/bt_connected.mp3" : "/bt_disconnected.mp3", AUDIO_GROUP_NONE );
    }

public:

    BluetoothState() : Debounce( false, BLUETOOTH_CHECK_PERIOD_FIRST_MS, BLUETOOTH_CHECK_PERIOD_MS, 0 )
    {
    }

};

