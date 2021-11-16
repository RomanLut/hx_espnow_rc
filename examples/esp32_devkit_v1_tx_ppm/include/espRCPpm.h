/*
 * This program is part an esp32 library to read and write RC protocols
 * Copyright (C) 2021  Pierre-Loup Martin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ESP_RC_CONTROLS_PPM_H
#define ESP_RC_CONTORLS_PPM_H

#include <Arduino.h>

#include "espRCControls.h"

class RCPpmIn : public RCControlsIn{
public:
	RCPpmIn(uint8_t pin, uint8_t timer, uint8_t numChannel = 8, uint16_t neutral = 3000);
	~RCPpmIn();
	
	bool update();

protected:
    static void _isrStatic();
    void _isr();

    static RCPpmIn* _pInstance;
    
	hw_timer_t *_timer;
	portMUX_TYPE _mux;
	int8_t _currentCh;
};

#endif