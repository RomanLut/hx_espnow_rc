# DIY RC controller

This is base project for building DIY RC controller, or modifying gamepad/RC controller.

RC controller can work in multiple modes. Controls are mapped to channels according to mapping defined in profiles.

[WORK IN PROGRESS]

- [x] Build on breadboard
- [x] Implement firmware
- [ ] Modify Dji Mini 1 RC Controller 



![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/diy_controller_prototype.jpg "DIY controller prototype")

# Connection diagram

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/diy_controller_connections.jpg "DIY controller connections")

# Parts list

- ESP32 DevkitC U32 or ESP32 Devkit V1 
- HC-06 bluetooth module
- DIY powerbank board
- Resistors: 4x 10K, 510Ohm, 1MOhm, 2MOhm
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
 
*Note: I do not recommend to use Arduino joysticks like KY-23 or RC controllers from toys. They have huge dead zones and low angles where values change.*

*I recommend to get original XBOX/Dualshock sticks ("3D Analog dualshock"), sticks from serious RC controllers, or modify original Dualshock/XBOX gamepads or RC controllers with good sticks.*

*Connections diagram is shown for 4-axis, 4-buttons RC controller. As there are few pins left, it is possible to add one axis and two buttons. Axes and buttons are configured in include\txConfig.h*


# Building guide

Pictures below provide hints for converting Mavic Mini 1 controller.

[TODO]

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/mavic_mini_rc/mavic_mini_sticks.jpg "Mavic Mini RC sticks")

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/mavic_mini_rc/mavic_cam_control.jpg "Mavic camera control")

**LB**, **RB** are **HOME** and **Power** buttons. **LT** and **RT** are **Record** and **Photo** buttons.

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/mavic_mini_rc/mavic_led_board.jpg  "Mavic LED board")

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/mavic_mini_rc/mavic_battery.jpg "Mavic battery")

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/mavic_mini_rc/mavic_mini_rc_1.jpg  "Mavic mini rc 1")

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/mavic_mini_rc/mavic_mini_rc_2.jpg  "Mavic mini rc 2")

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/mavic_mini_rc/mavic_mini_rc_3.jpg  "Mavic mini rc 3")

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/mavic_mini_rc/mavic_mini_rc_4.jpg  "Mavic mini rc 4")

Flash module with Examples/tx_diy_controller/

Upload filesystem Examples/tx_diy_controller/


# Sticks calibration

To enter **sticks calibration mode**, power the controller **with RIGHT Trigger pressed**. Controller will indicate calibration phase with LEDS and audio messages.

*Hold Right Bumper for ~3 seconds untill RC controller is powered completely.*

Alternatively, delete **calibration.json** to enter calibration mode.

**1. Min/max**

`LED pattern: *..* .**.` 

 Move all sticks and external axes to the min and max positions. Press **LEFT Bumper** to continue.


**2. Center**

 `LED pattern: .**.` 
 
 Move all sticks and external axis to center position. 

 Press **LEFT Bumper** to start calibratin. 
  
 Ligtly touch and move sticks and external axes in the range of backlash. RC controller will record this range of movement as central position.
  
 Press **LEFT Bumper** to finish calibratin. 
  
Calibration data is saved to **calibration.json**.

# Profile selection

To enable **Profile slection mode**, power controller **with LEFT Bumper pressed**.
Move left or right stick left and right to select profile 1...8.
LED pattern corresponds to profile number: 
- `1 *...` 
- `2 .*..` 
- `3 ..*.` 
- `4 ...*` 
- `5 .***` 
- `6 *.**` 
- `7 **.*` 
- `8 ***.`

Press **LEFT Bumper** to use selected profile. Selected profile will be stored and also used on next boot.


# Configration mode

To force **Configuration mode**, power the controller with **RIGHT Bumper** pressed.

**Configuration mode** is used to upload profiles to controller filesystem.

See **Configuration mode** https://github.com/RomanLut/hx_espnow_rc/blob/main/doc/mode_configuration.md

# See next

Profiles https://github.com/RomanLut/hx_espnow_rc/blob/main/doc/tx_profiles.md

Controls mapping https://github.com/RomanLut/hx_espnow_rc/blob/main/doc/mapping.md

