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

#include "espRCPpm.h"

RCPpmIn* RCPpmIn::_pInstance = nullptr;

RCPpmIn::RCPpmIn(uint8_t pin, uint8_t timer, uint8_t numChannel, uint16_t neutral){
	if(numChannel > 16) return;
	if(timer > 3) return;

	_data.channel = new int16_t[numChannel];
	_tempCh = new int16_t[numChannel];
	_data.numChannel = numChannel;

	memset(_data.channel, 0, sizeof(int16_t) * numChannel);
	memset(_tempCh, 0, sizeof(int16_t) * numChannel);

	_neutral = neutral;
	_resolution = 1024;
	_currentCh = 0;

	_pInstance = this;

	_mux = portMUX_INITIALIZER_UNLOCKED;

	pinMode(_pin, INPUT);

	_timer = timerBegin(timer, 40, true);
	attachInterrupt(pin, _isrStatic, RISING);

}

RCPpmIn::~RCPpmIn(){
	delete[] _data.channel;
	delete[] _tempCh;
}

bool RCPpmIn::update(){
	if(!_hasNew) return false;
	_hasNew = false;

	for (uint8_t i = 0; i < _data.numChannel; ++i){
		portENTER_CRITICAL_ISR(&_mux);
		_data.channel[i] = _tempCh[i] - _neutral;
		portEXIT_CRITICAL_ISR(&_mux);
	}

	return true;
}

// Protected methods

void RCPpmIn::_isrStatic(){
	if(_pInstance == nullptr) return;

	_pInstance->_isr();
}

void RCPpmIn::_isr(){
	uint64_t time = timerRead(_timer);
	timerRestart(_timer);
	if(time > 6000){
		_currentCh = 0;
		_hasNew = true;
	} else {
		portENTER_CRITICAL_ISR(&_mux);
		_tempCh[_currentCh] = (uint16_t)time;
		portEXIT_CRITICAL_ISR(&_mux);
		if(++_currentCh >= _data.numChannel){
			_currentCh--;
		}
	}

}