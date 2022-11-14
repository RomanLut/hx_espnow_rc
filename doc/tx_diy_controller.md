# DIY RC controller

This is base project for building DIY RC controller, or modifying gamepad/RC controller.

Example below shows how to convert Mavic Mini 1 RC controller.

Project Configuration can be adjusted for different axis and buttons count.

DIY RC controller can work in multiple modes: HX_ESPNOW_RC, Bluetooth gamepad and more.

**Goals**

- [x] Build on breadboard
- [x] Implement firmware
- [x] Modify Dji Mini 1 RC Controller 
- [x] Write full documentation and building guide
- [x] KY FPV mode
- [x] Xiro explorer mini mode
- [ ] Rize tello mode
- [ ] Eachine E58 mode


![alt text](/doc/diy_controller_prototype.jpg "DIY controller prototype")

# Connection diagram

![alt text](/doc/diy_controller_connections.jpg "DIY controller connections")

# Parts list

- ESP32 DevkitC U32 or ESP32 Devkit V1 
- HC-06 bluetooth module
- DIY powerbank board
- Resistors: 4x 10K, 510Ohm, 1MOhm, 2MOhm
- Variable resistor 1k (volume control, optional)
- 4 LEDs
- SMA to U.FL wire or Wifi antenna with feeder cable
- Sticks x2
- Buttons x4
- Switch x1
- self-centering variable resistor ( axis 4)
- speaker
- S8050 transistor or analogous NPN
- S8550 transistor or analogous PNP
- 47uF capacitor
- 100uF capacitor
- 47uH 0.37A inductance
 
*Note: I do not recommend to use Arduino joysticks like KY-23 or RC controllers from toys. They have huge dead zones and low angles where values change.*

*I recommend to get original XBOX/Dualshock sticks ("3D Analog dualshock"), sticks from serious RC controllers, or modify original Dualshock/XBOX gamepads or RC controllers with good sticks.*

*Connection diagram is shown for 5-axis, 4-buttons, 5 actions joystitck RC controller. Different configuration is possible. Axes and buttons are configured in include\txConfig.h*


# Building guide

Pictures below provide hints for converting Mavic Mini 1 RC Controller. We reuse sticks, buttons, camera control slider, original battery and micro USB connector.

Battery is charged with TP4056 board. DC-DC 5V boost converter is used to power ESP32 and Bluetooth module. 

USB 5V and USB GND lines of original microUSB connector are connected to the TP4056 board. USB D+ and USB D- lines are connected to the corresponding lines on ESP32 board.

*Note: DIY powerbank board was used in V1. It has one drawback though: when usb cable is disconnected, it takes ~1 second to start boost up converter. Due to this delay, controller reboots. I recommend to use TP4056 board and DC-DC boost converter.*

Mavic Mini controller can be disassembled using guides from youtube.

Connection diagrams for the internals of the controller are shown below:

![alt text](/doc/mavic_mini_rc/mavic_mini_sticks.jpg "Mavic Mini RC sticks")

![alt text](/doc/mavic_mini_rc/mavic_cam_control.jpg "Mavic camera control")

**LB**, **RB** are **HOME** and **Power** buttons. **LT** and **RT** are **Record** and **Photo** buttons.

![alt text](/doc/mavic_mini_rc/mavic_led_board.jpg  "Mavic LED board")

![alt text](/doc/mavic_mini_rc/mavic_battery.jpg "Mavic battery")

Solder ESP32, BT-06, powerbank module and amplifier according to schematics above.

![alt text](/doc/mavic_mini_rc/mavic_mini_rc_1.jpg  "Mavic mini rc 1")

![alt text](/doc/mavic_mini_rc/mavic_mini_rc_2.jpg  "Mavic mini rc 2")

Use control points on the original PCB to solder wires. Fix wires with epoxy glue.

![alt text](/doc/mavic_mini_rc/mavic_mini_rc_3.jpg  "Mavic mini rc 3")

![alt text](/doc/mavic_mini_rc/mavic_mini_rc_4.jpg  "Mavic mini rc 4")

Remove PCB antenna from bluetooth module. Cut U.FL connector from feeder wire and solder right antenna of RC controller to the module:

![alt text](/doc/mavic_mini_rc/mavic_mini_rc_bt_1.jpg  "Mavic mini bt 1")

![alt text](/doc/mavic_mini_rc/mavic_mini_rc_bt_2.jpg  "Mavic mini bt 2")

Fix feeder cable with epoxy glue:

![alt text](/doc/mavic_mini_rc/mavic_mini_rc_bt_3.jpg  "Mavic mini bt 3")

Remove PCB antenna from ESP32 module and solder left antenna of RC controller:

![alt text](/doc/mavic_mini_rc/mavic_mini_rc_wifi_1.jpg  "Mavic mini wifi")

Volume control if placed on the front cover of the controller (fixed with epoxy glue). The wheel is covered by antennas in folded state.

![alt text](/doc/mavic_mini_rc/mavic_mini_ant.jpg  "Mavic mini ant")

I also used Joystick from Siemens SX65 mobile phone. It has 4 direction buttons and one center button.

![alt text](/doc/mavic_mini_rc/mavic_mini_rc_joy_1.jpg  "Mavic mini rc joy 1")

![alt text](/doc/mavic_mini_rc/mavic_mini_rc_joy_2.jpg  "Mavic mini rc joy 2")

![alt text](/doc/mavic_mini_rc/mavic_mini_rc_joy_3.jpg  "Mavic mini rc joy 3")

Charging indicator LED and power switch are placed on the back cover of the RC controller (fixed with epoxy glue):

![alt text](/doc/mavic_mini_rc/mavic_mini_rc_7.jpg  "Mavic mini rc 7")

![alt text](/doc/mavic_mini_rc/mavic_mini_rc_5.jpg  "Mavic mini rc 5")

![alt text](/doc/mavic_mini_rc/mavic_mini_rc_6.jpg  "Mavic mini rc 6")

In version 2, I used tiny  speaker from some mobile phone. It can be mouted against holes of the bottom cover, producing good sound:

![alt text](/doc/mavic_mini_rc/mavic_mini_rc_speaker_1.jpg  "Mavic mini rc speaker 1")

![alt text](/doc/mavic_mini_rc/mavic_mini_rc_speaker_2.jpg  "Mavic mini rc speaker 2")

![alt text](/doc/mavic_mini_rc/diy_mavic_mini_rc_mod.jpg  "Mavic mini rc 8")

Flash module with Examples/tx_diy_controller/

Upload filesystem Examples/tx_diy_controller/ (on the left side of the screen in PlatromIO IDE, select PlatformIO -> Uload filesystem image)


# Sticks calibration

To enter **sticks calibration mode**, power the controller **with RIGHT Bumper pressed**. Controller will indicate calibration phase with LEDS and audio messages.

*Hold Right Bumper for ~3 seconds untill RC controller is powered completely.*

Alternatively, delete **calibration.json** to enter calibration mode.

**1. Min/max**

`LED pattern: *..* .**.` 

 Move all sticks and external axes to the min and max positions. Press **LEFT Bumper** to continue.


**2. Center**

 `LED pattern: .**.` 
 
 Move all sticks and external axis to center position. If non-centering sliders are used on axes, more approximatelly to center position.

 Press **LEFT Bumper** to start calibratin. 
  
 Ligtly touch and move sticks and external axes in the range of backlash. RC controller will record this range of movement as central position.
  
 Press **LEFT Bumper** to finish calibratin. 
  
Calibration data is saved to **calibration.json**.

# Profile selection

To enable **Profile slection mode**, power controller **with LEFT Trigger pressed**.
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

To force **Configuration mode**, power the controller with **RIGHT Trigger** pressed.

**Configuration mode** is used to upload profiles to controller filesystem.

See **Configuration mode** [/doc/mode_configuration.md](/doc/mode_configuration.md)


# See next

Profiles [/doc/tx_profiles.md](/doc/tx_profiles.md)

Controls mapping [doc/mapping.md](doc/mapping.md)

