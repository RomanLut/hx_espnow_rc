
# Mapping

Controller has to map available inputs ( axes, buttons, sliders ) to channels.

Mapping is described in profile json:

```json
{
	"transmitter_mode": "ESPNOW",
	"espnow_channel": 3,
	"espnow_key": 0,
	"espnow_long_range_mode": false,
	"ap_name": "hxrct",
	"ap_password": "",
	"mapping": [
		{ "event": { "name": "STARTUP"	},	"op": {	"name": "SOUND",	"parm": "/mode_esp_now.mp3"		} },

		{ "event": { "name": "ALWAYS"	},	"op": {	"name": "AXIS",		"parm": "RIGHT_STICK_X", 	"channel": 1 	} },
		{ "event": { "name": "ALWAYS"	},	"op": {	"name": "AXIS",		"parm": "RIGHT_STICK_Y", 	"channel": 2 	} },
		{ "event": { "name": "ALWAYS"	},	"op": {	"name": "AXIS",		"parm": "LEFT_STICK_Y",		"channel": 3 	} },
		{ "event": { "name": "ALWAYS"	},	"op": {	"name": "AXIS",		"parm": "LEFT_STICK_X",		"channel": 4 	} },
		{ "event": { "name": "ALWAYS"	},	"op": {	"name": "SWITCH",	"parm": "LEFT_BUMPER", 		"channel": 5 	} },
		{ "event": { "name": "ALWAYS"	},	"op": {	"name": "SWITCH",	"parm": "RIGHT_BUMPER",		"channel": 6 	} },

		{ "event": { "name": "CHANNEL_EQUAL", "channel": 5, "value": 1000, "once": "yes" }, "op": { "name": "SOUND", "parm": "/disarmed.mp3"    } },
		{ "event": { "name": "CHANNEL_EQUAL", "channel": 5, "value": 2000, "once": "yes" }, "op": { "name": "SOUND", "parm": "/armed.mp3"       } }
	]
}
```

*Mapping is used in **DIY RC controller**. Mapping definition is ignored in **External transmitter module**. In **External transmitter module**, channels mapping is defined using OpenTX functionality.*

Mapping is a set of actions executed one-by-one on each loop:

`{ "event": { "name": "ALWAYS"	},	"op": {	"name": "AXIS",		"parm": "RIGHT_STICK_X", 	"channel": 1 	} }`

Action consists of **event** and operation **op**.

If mapping is not provided, controller will map all Axes and buttons to channels consecutive.

Buttons will be mapped starting from channel `BLE_GAMEPAD_AXIS_COUNT` ( ==6 ).

# event

Event definition includes name of event, and event specific parameters:

`"event": { "name": "CHANNEL_EQUAL", "channel": 5, "value": 2000, "once": "yes" }`

#### STARTUP

Action is executed one time on profile initialization.
For example, it can be used to assign values for channels using `CONTSTANT` input, or play audio message.

`{ "event": { "name": "STARTUP"	},	"op": {	"name": "SOUND", "parm": "/mode_esp_now.mp3" } },`
 
On statup, all channels have value 1000. `STARTUP` is the only action which is executed on the first loop.

#### ALWAYS

 Action is executed on each loop.

#### BUTTON_PRESS

`"event": { "name": "BUTTON_PRESS", "parm": "LEFT_BUMPER" }`

Action is executed when specified physical button is pressed.

**parm** is button name.

#### CHANNEL_EQUAL

`"event": { "name": "CHANNEL_EQUAL", "channel": 5, "value": 1000, "once": "yes" }`

Channel value is compared with "value".

**parm** is value. Value is specified without quotes.

**channel** is target channel.

**once** is either "no"" or "yes".

This action is executed on each loop ("once":"no"), or each time channel value is changed ("once": "yes") (action is executed one time after each change). 

Make sure action is placed after (not before) action which actually changes channel value, otherwise change will not be detected.

Common usage of "ence" action is to play sound file using `SOUND` action.


# op

`"op": { "name": "AXIS", "parm": "RIGHT_STICK_X", "channel": 1 }` 

Operation definition contains operation name, and operation specific parameters:

#### AXIS

`"op": { "name": "AXIS", "parm": "RIGHT_STICK_X", "channel": 1 }`

This actions sets `AXIS` value to the channel.

**parm** is axis name.

**channel** is target channel.


#### BUTTON

`"op" : { "op" : "BUTTON", "parm" : "LEFT_THUMB", "channel" : 5 }`

This action sets channel value to 1000/2000 depending on button/switch status.

**parm** is button name.

**channel** is target channel.


#### SWITCH, SWITCH3, SWITCH3, SWITCH

`"op": { "name" : "SWITCH", "parm" : "LEFT_THUMB", "channel" : 5 }`

This action is used to implement N-position switch using a button. Channel value is switched between step values on each button press.
- SWITCH: 1000/2000
- SWITCH3: 1000/1500/2000
- SWITCH4: 1000/1333/1666/2000
- SWITCH6: 1000/1250/1500/1750/2000
- SWITCH16: 16 positions. Can be used for INAV adjustments.

**parm** is button name.

**channel** is target channel.


#### AXIS_SWITCH

`"op": { "name" : "AXIS_SWITCH", "parm" : "AXIS4", "channel" : 5 }`

This action is used to implement switch using self-sentering axis. Channel value is switched between 1000/2000 values when axis is moved left or right.

**parm** is axis name.

**channel** is target channel.


#### AXIS_BUTTON_LEFT and AXIS_BUTTON_RIGHT, AXIS_BUTTON_DOWN and AXIS_BUTTON_UP

`"op": { "name" : "AXIS_BUTTON_LEFT", "parm" : "AXIS4", "channel" : 5 }`

`"op": { "name" : "AXIS_BUTTON_RIGHT", "parm" : "AXIS4", "channel" : 6 }`

Theese actions are used to implement button using self-centering axis. Channel value is switched between 1000/2000 values while axis is hold at specified direction.

This way axis works as two independent buttons.

**parm** is axis name.

**channel** is target channel.


#### AXIS_SWITCH_LEFT and AXIS_SWITCH_RIGHT, AXIS_SWITCH_DOWN and AXIS_SWITCH_TOP

`"op": { "name" : "AXIS_SWITCH_LEFT", "parm" : "AXIS4", "channel" : 5 }`

`"op": { "name" : "AXIS_SWITCH_RIGHT", "parm" : "AXIS4", "channel" : 6 }`

Theese actions are used to implement switches using celf-sentering axis. Channel value is switched between 1000/2000 values when axis is moved to specified direction.

This way axis works as two independent switches.

**parm** is axis name.

**channel** is target channel.


#### ADDITIVE

 `"op:" { "name"" : "ADDITIVE", "parm" : "LEFT_STICK_Y", speed: 5, "channel" : 1 }`
 
 `CH += (Axis - 1500) * dT * speed / 10000;`
 
 If Throttle axis is self-centering, sometimes it is desired to implement additive axis.
 
 When stick is moved up, value of channel increases proportionally.
 
 When stick is moved down, value of channel decreases proportionally.
 
 **speed** is amount of axis value to add to the channel value. Specified without quotes.
 
 **speed** value of 1 means that channel will reach maximum in 10 seconds, if stick is moved to maximum.
 
 **speed** value of 10 means that channel will reach maximum in 1 second, if stick is moved to maximum.

 
#### CONSTANT

`"op": { "name" : "CONSTANT", "parm" : 1500, "channel" : 5 },`

This action is used to assign constant value to the channel.

**parm** is value. Value is specified without quotes.

**channel** is target channel.


#### MULD10

`"op": { "name" : "MULD10", "parm" : -1, "channel" : 1  },`

`CH = (CH - 1500) * MUL / 10 + 1500.`

This action is used to multiply channel value.

**parm** is multiplicator (integer). Value is specified without quotes. Value 1 means "multiply by 1/10".

**channel** is target channel.

Can be used to inverse axis (multiply by -1) or map upper part of axis to full range.


#### ADD

`"op": { "name" : "ADD", "parm" : 500, "channel" : 1 },`

`CH = CH + value`

This action is used to add value to the channel.

**parm** is value (integer). Value is specified without quotes.

**channel** is target channel.

#### EXPO

`"op": { "name" : "EXPO", "parm" : 20, "channel" : 1 },`


This action sets Expo curve value for a channel.

**parm** is expo curve coefficient (integer). Value is specified without quotes. Range in 0..100. 0 menas linear, 100 means cubic.

**channel** is target channel.


#### TRIM

`"op": { "name": "TRIM", "parm": "LEFT_TRIGGER", "channel": 8 } },`

This action is used to trim sticks: Left X, Right X, Right Y. Move sticks while holding modifier button to adjust trim.

Trim is reset on reboot.

**parm** is modifier button name.


#### SOUND

`"op": { name : "SOUND", "parm" : "/armed.mp3" },`

This action used to play sound file.

**parm**" is sound file in mp3 or wav format. Leading "/" is required.

MP3: 24KHz, momo, 128-192kBit

WAV: 8-16KHz 8bit mono.

# Axis names

Available axis names:

**AXIS0 == LEFT_STICK_X**

**AXIS1 == LEFT_STICK_Y**

**AXIS2 == RIGHT_STICK_X**

**AXIS3 == RIGHT_STICK_Y**

**AXIS4 == Self-centering jog wheel**


# Button names

Available button names:

**BUTTON0 == LEFT_BUMPER**

**BUTTON1 == RIGHT_BUMPER**

**BUTTON2 == LEFT_TRIGGER**

**BUTTON3 == RIGHT_TRIGGER**

**BUTTON4 == JOY_CENTER**

**BUTTON5 == JOY_LEFT**

**BUTTON6 == JOY_RIGHT**

**BUTTON7 == JOY_UP**

**BUTTON8 == JOY_DOWN**


# Inverting axis

 To inverse axis, multiply channel by -1 after setting axis value:

```
 { "event": { "name" :"ALWAYS" }, "op": { "name" : "AXIS", "parm" : "LEFT_STICK_X", "channel" : 1 } },
 { "event": { "name": "ALWAYS" }, "op": { "name" : "MUL", "parm" : -10, "channel" : 1 } }
```

# Mapping upper part of axis to the full range

 If Throttle axis is self-centering, sometimes it is desired to map apper part of axis to full range.
```
 { "event": { "name": "ALWAYS" }, "op": { "name" : "AXIS", "parm" : "LEFT_STICK_Y", "channel" : 1 },
 { "event": { "name": "ALWAYS" }, "op": { "name" : "MUL", "parm" : 20, "channel" : 1 },
 { "event": { "name": "ALWAYS" }, "op": { "name" : "ADD", "parm" : -500, "channel" : 1 }
```

# Implementing 5-position switch with joystick buttons

 Joystick or any other set of buttons can be used to implement multiple position switch.
```
  { "event": { "name": "STARTUP" },  "op": { "name": "CONSTANT", "parm": 1000, "channel": 9 } },

  { "event": { "name": "BUTTON_PRESS", "parm": "JOY_CENTER" }, "op": {	"name": "CONSTANT", "parm": "1200", "channel": 9 } },
  { "event": { "name": "BUTTON_PRESS", "parm": "JOY_LEFT"   }, "op": {	"name": "CONSTANT", "parm": "1400", "channel": 9 } },
  { "event": { "name": "BUTTON_PRESS", "parm": "JOY_RIGHT"  }, "op": {	"name": "CONSTANT", "parm": "1600", "channel": 9 } },
  { "event": { "name": "BUTTON_PRESS", "parm": "JOY_UP"     }, "op": {	"name": "CONSTANT", "parm": "1700", "channel": 9 } },
  { "event": { "name": "BUTTON_PRESS", "parm": "JOY_DOWN"   }, "op": {	"name": "CONSTANT", "parm": "1800", "channel": 9 } },	
```

# Clamping 
Each channel value is clamped to the range 1000...2000 after completing all operations.

Intermediate values can be in range -32768...32768.

# Comments
Json format does not support comments. To add comments, use any unused parameter name ( unknown parameters are ignored) :

`{ "event": { "name": "ALWAYS" }, op": { "name": "BUTTON", "parm": "LEFT_BUMPER", "channel": 5,	"note":"takeoff" } }`
