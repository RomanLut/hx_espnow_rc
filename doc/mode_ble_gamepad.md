# BLE gamepad


Profile emulates BLE gamepad. Name is HXRCBLE. 
Gamepad can be used with PC or Android.

# Channel mapping

Channels 1...6 are output as X,Y,Z,RX,RX,RZ axes of gamepad. 

Channels 7...15 are mapped to gamepad buttons 1-9 (<1750 - unpressed, >= 1750 - pressed).

Example profile:
``
{
    "transmitter_mode" : "BLEGAMEPAD"
}
``