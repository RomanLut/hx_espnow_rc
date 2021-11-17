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

#ifndef ESP_RC_CONTROLS_H
#define ESP_RC_CONTROLS_H

#include <Arduino.h>

class RCControls{
public:

	struct controlData_t{
		int16_t *channel;
		uint8_t numChannel;
	};

	virtual bool update() = 0;

protected:
	controlData_t _data;
	int16_t _resolution;
	int16_t _neutral;
	uint8_t _pin;
	int16_t *_tempCh;
	volatile bool _hasNew;
};

class RCControlsIn : public RCControls{
public:
	virtual bool update() = 0;

	controlData_t* getChannels();
	int16_t getChannel(uint8_t channel);
	uint8_t getNumberChannels();
	
	int16_t getResolution();
	int16_t getNeutral();

private:

};

class RCControlsOut : public RCControls{
public:
	virtual bool update() = 0;

	void setChannels(controlData_t *data);
	void setChannel(uint8_t channel, int16_t value);
	void setNumberChannels(int16_t numberChannel);

private:

};

#endif