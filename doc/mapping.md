
# Mapping

Controller has to map available inputs ( axes, buttons, sliders ) to channels.

Mapping is described in profile json:

```json
{
    "transmitter_mode" : "ESPNOW",
    "espnow_channel" : 3,
    "espnow_key" : 0,
    "espnow_long_range_mode" : false,
    "ap_name" : "hxrct",
    "ap_password" : "",
    "mapping":[
        { "event" :"ALWAYS", "channel" : 1, "op" : "AXIS", "parm" : "LEFT_STICK_X" },
        { "event" :"ALWAYS", "channel" : 2, "op" : "AXIS", "parm" : "LEFT_STICK_Y" },
        { "event" :"ALWAYS", "channel" : 3, "op" : "AXIS", "parm" : "RIGHT_STICK_X" },
        { "event" :"ALWAYS", "channel" : 4, "op" : "AXIS", "parm" : "RIGHT_STICK_Y" },
        { "event" :"ALWAYS", "channel" : 5, "op" : "TRIGGER", "parm" : "LEFT_BUMPER" },
        { "event" :"ALWAYS", "channel" : 6, "op" : "TRIGGER", "parm" : "RIGHT_BUMPER" }
    ]
}
```

*Mapping is used in **DIY RC controller**. Mapping definition is ignored in **External transmitter module**. In **External transmitter module**, channels mapping is defined using OpenTX functionality.*

Mapping is a set of actions executed one-by-one on each loop:

`{ "event" :"ALWAYS", "channel" : "1", "op" : "AXIS", "parm" : "LEFT_STICK_X" }`

If mapping is not provided, controller will map all Axesr and buttons to channels consecutive.

Buttons will be mapped starting from channel `BLE_GAMEPAD_AXIS_COUNT` ( ==6 ).

# event

#### STARTUP

Action is executed one time on profile initialization.
It can be used to assign values for channels using `CONTSTANT` input.
 
On statup, all channels have value 1000. `STARTUP` is the only action which is executed on the first loop.

#### ALWAYS

 Action is executed on each loop.

#### CHANNEL_EQUAL_1000 
#### CHANNEL_EQUAL_1333
#### CHANNEL_EQUAL_1500 
#### CHANNEL_EQUAL_1666
#### CHANNEL_EQUAL_2000

Theese actions are executed each time channel value is changed to specified value ( action is executed one time after each change ).
Make sure action is placed after (not before) action which actially changes channel value, otherwise change will not be detected.
Common usage is to play sound file with `SOUND` action.


# channel 

 **Channel** is target channel for this action, 1..15

# op

 `op` is operation to run on selected channel:

#### AXIS

This actions sets `AXIS` value to the channel.

**parm** is axis: **LEFT_STICK_X == AXIS0, LEFT_STICK_Y == AXIS1, RIGHT_STICK_X == AXIS2, RIGHT_STICK_Y == AXIS3, AXIS4...AXIS15**

`{ "event" :"ALWAYS", "channel" : 1, "op" : "AXIS", "parm" : "LEFT_STICK_X" }`

#### BUTTON

This action sets channels value to 1000/2000 depending on button/switch status.

parm is button: **LEFT_BUMPER == BUTTON0, RIGHT_BUMPER == BUTTON1, BUTTON2 ... BUTTONN**

`{ "event" :"ALWAYS", "channel" : 5, "op" : "BUTTON", "parm" : "LEFT_THUMB" }`

#### TRIGGER

This action is used to implement switch using button. Channel value is switched between 1000/2000 on each button press.

**parm** is button name.

`{ "event" :"ALWAYS", "channel" : 5, "op" : "TRIGGER", "parm" : "LEFT_THUMB" }`

#### SWITCH3

This action is used to implement 3-state switch using button. Channel value is switched beetwen 1000/1500/2000 on each button press.

**parm** is button name.

`{ "event" :"ALWAYS", "channel" : 5, "op" : "SWITCH3", "parm" : "LEFT_THUMB" }`

#### SWITCH4

This action is used to implement 4-state switch using button. Channel value is switched beetwen 1000/1333/1666/2000 on each button press.

**parm** is button name.

`{ "event" :"ALWAYS", "channel" : 5, "op" : "SWITCH4", "parm" : "LEFT_THUMB" }`

#### CONSTANT

This action is used to assign constant value to the channel

**parm** is value

Note that value is specified without quotes.

`{ "event" :"STARTUP", "channel" : 5, "op" : "CONSTANT", "parm" : 1500 },`

#### MUL

This action is used to multiply channel value.

`CH = (CH - 1500) * MUL / 10 + 1500.`

Can be used to inverse axis (multiply by -1) or map upper part of axis to full range.

**parm** is multiplicator (integer)

Note that value is specified without quotes.

Note that value 1 means "multiply by 1/10".

`{ "event" :"ALWAYS", "channel" : 1, "op" : "MUL", "parm" : -1 },`

#### ADD

This action is used to add value to channel.

`CH = CH + value`

**parm** is value (integer)

Note that value is specified without quotes.

- `{ "event" :"ALWAYS", "channel" : 1, "op" : "ADD", "parm" : 500 },`

#### SOUND

This action does not modify channels value. It is used to play sound file.

`{ "event" :"EQUAL_1000", "channel" : 5, "op" : "SOUND", "parm" : "/armed.wav" },`



# Inversing axis

 To inverse axis, multiply channel by -1 after setting axis value:

```
 { "event" :"ALWAYS", "channel" : 1, "op" : "AXIS", "parm" : "LEFT_STICK_X" },
 { "event" :"ALWAYS", "channel" : 1, "op" : "MUL", "parm" : -10 }
```

# Mapping upper part of axis to the full range

 If Throttle axis is self-centering, sometimes it is desired to map apper part of axis to full range.
```
 { "event" :"ALWAYS", "channel" : 1, "op" : "AXIS", "parm" : "LEFT_STICK_Y" },
 { "event" :"ALWAYS", "channel" : 1, "op" : "MUL", "parm" : 20 },
 { "event" :"ALWAYS", "channel" : 1, "op" : "ADD", "parm" : -500 }
```

# Additive axis

 If Throttle axis is self-centering, sometimes it is desired to implement additive axis.
 
 When stick is moved up, value of channel increases proportionally.
 
 When stick is moved down, value of channel decreases proportionally.
 
 **speed** is amount of axis value to add to the channel value.
 
 `CH += (Axis - 1500) * dT * speed / 10000;`
 
 **speed** value of 1 means that channel will reach maximum in 10 seconds, if stick is moved to maximum.
 
 **speed** value of 10 means that channel will reach maximum in 1 second, if stick is moved to maximum.

 `{ "event" :"ALWAYS", "channel" : 1, "op" : "ADDITIVE", "parm" : "LEFT_STICK_Y", speed: 5 }`

# Clamping 
 Each channel value is clamped to the range 1000...2000 after completing all operations.
 
 Intermediate values can be in range -32768...32768.
