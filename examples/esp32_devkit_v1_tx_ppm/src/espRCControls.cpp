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

#include "espRCControls.h"

RCControls::controlData_t* RCControlsIn::getChannels(){
	return &_data;
}

int16_t RCControlsIn::getChannel(uint8_t channel){
	if(channel > _data.numChannel) return 0;
	return _data.channel[channel];
}

uint8_t RCControlsIn::getNumberChannels(){
	return _data.numChannel;
}

int16_t RCControlsIn::getResolution(){
	return _resolution;
}

int16_t RCControlsIn::getNeutral(){
	return _neutral;
}
