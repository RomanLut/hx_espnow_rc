#pragma once

/*

This project routes mavlink to USB Serial, and emulates usb keyboard.
Used in Oculus Quest 2 GS in hx-esp32cam-fpv project.

Pinout:

ESP32-S3 Super Mini, component side, USB-C at top
Buttons are active LOW: each button connects its GPIO to GND when pressed.
                                              [       ]
                                        +-----[       ]-----+
 optional MAVLink UART TX       TX/G43  |     [ USB-C ]     |  5V          USB/input 5V
 optional MAVLink UART RX       RX/G44  |                   |  GND         button common ground
 Enter KEY                      GPIO1   |                   |  3V3         3.3V out, unused here
 R KEY                          GPIO2   |                   |  GPIO13      unused
 G KEY                          GPIO3   |      ESP32-S3     |  GPIO12      unused
 UP button -> GND               GPIO4   |     Super Mini    |  GPIO11      unused
 DOWN button -> GND             GPIO5   |                   |  GPIO10      unused
 LEFT button -> GND             GPIO6   |                   |  GPIO9       unused
 RIGHT button -> GND            GPIO7   |                   |  GPIO8       unused
                                        +-------------------+

MAVLink uses native USB CDC when USE_USB_CDC == true.
TX/GPIO43 and RX/GPIO44 are used only when USE_USB_CDC == false.

To enter USB Flash mode, hold R kay during powerup.
*/

// Route MAVLink through native USB CDC (virtual COM port on USB connector).
#define USE_USB_CDC true

// MAVLink serial port pins (used only when USE_USB_CDC == false)
#define MAVLINK_RX_PIN    44
#define MAVLINK_TX_PIN    43

// USB keyboard GPIO mapping (active LOW, internal pull-up enabled)
#define KEY_UP_GPIO       4
#define KEY_DOWN_GPIO     5
#define KEY_LEFT_GPIO     6
#define KEY_RIGHT_GPIO    7
#define KEY_ENTER_GPIO    1
#define KEY_R_GPIO        2
#define KEY_G_GPIO        3

//Use Mavlink v1 ( 8 RC Channels ) or Mavlink v2 (15 RC Channels)
#define USE_MAVLINK_V1 false

//telemetry/mavlink port speed
#define TELEMETRY_BAUDRATE 115200

//send packet every ?ms
//Note for inav: "Serial receiver half-duplex" should ne set to "OFF", otherwise inav will not be able to send telemetry.
#define MAVLINK_RC_PACKET_RATE_MS   40  //Rate in Hz = 1000 / MAVLINK_RC_PACKET_RATE_MS

//note: 
//Mavlink v1: 18 bytes(message size) * 25(rate) * 10(bits) = 4500 baud
//Mavlink v2: 34 bytes(message size) * 25(rate) * 10(bits) = 8500 baud

//=============================================================================
//Receiver binding
#define USE_WIFI_CHANNEL 3
#define USE_KEY 0 

#define USE_LR_MODE false

//if there is not transmitter connection after powerup to the specified time,
//receiver will switch from LR to nomal mode to show AP and allow OTA updates
//set to 0 to disable
#define NORMAL_MODE_DELAY_MS 60*1000
