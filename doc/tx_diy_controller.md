# DIY RC controller

This is base project for building DIY RC controller, or modifying gamepad/RC controller.

[TODO]

Controller can work in multiple modes:
- ESP_NOW DIY RC system
- WIFI controlled drones
- BLE Gamepad.

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/diy_controller_prototype.jpg "DIY controller prototype")

# Parts list

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/diy_controller_connections.jpg "DIY controller connections")


- ESP32 DevkitC U32 or ESP32 Devkit V1 
- HC-06 bluetooth module
- DIY powerbank board
- Resistors: 10K 4x, 510Ohm, 1MHom, 2MOhm
- variable resistor 1k
- 4 LEDs
- SMA to U.FL wire or Wifi antenna with feeder cable
- Sticks x2
- Buttons x4
- Switch x1
- speaker
- S8050 transistor or analogous NPN
- S8550 transistor or analogous PNP
- 47uF capacitor
- 100uF capacitor
- 47uH 0.37A inductance
 
*Note: I do not recommend to use Arduino joysticks like KY-23 or RC controllers from toys. They have huge dead zones and low angles whenre values change.*
*I recommend to get original XBOX/Dualshock joysticks ("3D Analog dualshock"), joysticks from serious RC controllers, modify original Dualshock/XBOX gamepads, or modify RC controllers with good joysticks.*

*Connections diagram is shown for 4-axis, 4-buttons RC controller. As there are few pins left, it is possible to add one axis and two buttons. Axes and buttons are configured in include\txConfig.h*


# Building guide

[TODO]

Flash module with Examples/tx_diy_controller/

Upload filesystem Examples/tx_diy_controller/


#Sticks calibration

To enter sticks calibration mode, power the controller with left stick moved LEFT DOWN. Controller will indicate calibration phase with LEDS and audio messages.

1) LED pattern: *..* .**. Move all sticks and external axes to the min and max positions.
2) LED pattern: .**. Move all sticks and external axis to center position.

# Profile selection

To select profile, power controller with LEFT Bumper pressed.
Move left or right stick left and right to select profile 1...8.
LED patterns: 
1 *... 
2 .*.. 
3 ..*. 
4 ...* 
5 .*** 
6 *.** 
7 **.* 
8 ***. 

Press LEFT bumper to use selected profile. Selected profile will be stored and also used on next boot.


# Configration mode

To force configuration mode, power the controller with RIGHT bumper pressed.

Configuration mode is used to upload profiles to controller filesystem.
In this mode, controller is creating Wifi access point and FTP server.
FTP server can be used to edit/upload profile json files.

``
wifi ap: hxrct
wifi password: [empty]
ftp user: anonymous
ftp password: anonymous
``

To change theese settings, edit Configuration mode profile (profile10.json).

# See next

Profiles https://github.com/RomanLut/hx_espnow_rc/blob/main/doc/tx_profiles.md

