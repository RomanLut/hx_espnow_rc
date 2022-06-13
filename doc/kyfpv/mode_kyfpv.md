# KY FPV mode

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/kyfpv/ky_fpv_icon.png "KY FPV icon")

This mode is used to control drones which are controlled by KY FPV application, like E58 clone.

KY FPV protocol is different from Eachine E58 protocol described here: https://github.com/martin-ger/ESP_E58-Drone

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/kyfpv/e58.jpg "E58 drone")


# Protocol details

Drone can be controlled by it's RC controller or by Android application.

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/kyfpv/ky_fpv_screen.jpg "KY FPV app")

Camera board is separated from Flight controller.

Flight controller is based on microcontroller which has 2.4Ghz receiver. It does not use Wifi protocol.

Camera board contains Wifi module. Camera board setups AP and streams video using UDP packets. Protocol is unknown.

There is on-way UART connection from camera board to flight controller. RC control packets can be sent to camera board, which are routed to UART.

RC Control packets should be send every 50ms. 

Camera board listen on UDP Port 8800. Many details about packet format are unknown. Packet can contain multiple commands to control streaming. RC control packet is one of comands which can be included in the packet.

Mode is implemented by replaying packet which contains RC Control data. This data includes stick positions, flags bitfield for commands and packets counter.

On connection, special packet is sent untill camera board reply. This packet is also used to reset packets counter, otherwise link can not sync.

Takeof, landing, emergency stop, gyro calibrate commands are sent for 0.5 second.


# Channel mapping

Channels 1...4: AETR

Channel 5: >=1750 - takeoff

Channel 6: >=1750 - landing

Channel 7: >=1750 - emergency stop

Channel 8: >=1750 - headless mode, > 1750 - mode enabled ( drone blinks LEDS 2 times repetitevely )

Channel 9: >=1750 - flip modifier ( roll left or right while holding flip button )

Channel 10: >=1750 - calibrate gyro (drone blinks LEDS ~20 Hz for 1 second)


Example mapping in DIY controller:

LEFT BUMPER - takeoff

RIGHT BUMPER - landing

RIGHT TRIGGER - emergency stop

LEFT TRIGGER - headless mode, also TRIM modifier

AXIS4 left - flip modifier

AXIS4 right - gyro calibration

```
{
   "transmitter_mode": "KYFPV",
   "mapping": [
      { "event": { "name": "STARTUP"            }, "op": { "name": "SOUND",	"parm": "/mode_KYFPV.mp3"                     } },
      { "event": { "name": "CONNECTED"          }, "op": { "name": "SOUND",	"parm": "/connected.mp3"                      } },
      { "event": { "name": "DISCONNECTED"       }, "op": { "name": "SOUND",	"parm": "/disconnected.mp3"                   } },
      { "event": { "name": "TAKEOFF"            }, "op": { "name": "SOUND",	"parm": "/takeoff.mp3"                        } },
      { "event": { "name": "LANDING"            }, "op": { "name": "SOUND",	"parm": "/landing.mp3"                        } },
      { "event": { "name": "EMERGENCY_STOP"     }, "op": { "name": "SOUND",	"parm": "/emergency_stop.mp3"                 } },
      { "event": { "name": "HEADLESS_ENABLED"	}, "op": { "name": "SOUND",	"parm": "/headless_mode_on.mp3"               } },
      { "event": { "name": "HEADLESS_DISABLED"  }, "op": { "name": "SOUND",	"parm": "/headless_mode_off.mp3"              } },
      { "event": { "name": "GYRO_CALIBRATION"	}, "op": { "name": "SOUND",	"parm": "/gyro_calibration.mp3"               } },
      { "event": { "name": "BEEP_SHORT"         }, "op": { "name": "SOUND",	"parm": "/beep_short.mp3", "note":"flip mode" } },

      { "event": { "name": "ALWAYS" }, "op": { "name": "AXIS", "parm": "RIGHT_STICK_X", "channel": 1 } },
      { "event": { "name": "ALWAYS" }, "op": { "name": "AXIS", "parm": "RIGHT_STICK_Y", "channel": 2 } },
      { "event": { "name": "ALWAYS" }, "op": { "name": "AXIS", "parm": "LEFT_STICK_Y",  "channel": 3 } },
      { "event": { "name": "ALWAYS" }, "op": { "name": "AXIS", "parm": "LEFT_STICK_X",  "channel": 4 } },

      { "event": { "name": "ALWAYS" }, "op": { "name": "BUTTON", "parm": "LEFT_BUMPER",   "channel": 5, "note":"takeoff"        } },
      { "event": { "name": "ALWAYS" }, "op": { "name": "BUTTON", "parm": "RIGHT_BUMPER",  "channel": 6, "note":"landing"        } },
      { "event": { "name": "ALWAYS" }, "op": { "name": "BUTTON", "parm": "RIGHT_TRIGGER", "channel": 7, "note":"emergency stop" } },

      { "event": { "name": "ALWAYS" }, "op": { "name": "AXIS_BUTTON_RIGHT", "parm": "AXIS4", "channel": 8, "note":"headless mode" } },
      { "event": { "name": "ALWAYS" }, "op": { "name": "AXIS_BUTTON_LEFT",  "parm": "AXIS4", "channel": 9, "note":"flip mode"     } },
      
      { "event": { "name": "ALWAYS" }, "op": { "name": "TRIM", "parm": "LEFT_TRIGGER", "note":"trim modifier" } }
   ]
}
```

# Telemetry

RSSI SPORT sensor shows Wifi network RSSI (23 means -23dbm).

Battery level is not available unfotunately.

# Known problems

1) Drone does not allow to associate more then one Wifi client by default.

But it is still possible to fly with FPV with a trick:

- turn on drone

- connect phone to drone

- turn off phone wifi

- turn on RC Controller, connect to drone

- connect phone to drone (turn on wifi). May need to retry reconenction 4-5 times untill success.


# Links

- Controlling Eachine E58 https://github.com/martin-ger/ESP_E58-Drone
- Hacking Chinese Drones for Fun and (No) Profit https://blog.horner.tj/hacking-chinese-drones-for-fun-and-no-profit/
- Hacking_Drone_Eachine-e58 https://github.com/OrShemesh1992/Hacking_Drone_Eachine-e58
- WiFi UFO Drone https://github.com/LukasMaly/wifi-ufo-drone
- Eachine E58 CFW https://github.com/flaminggoat/Eachine-E58-CFW
- lewei camera https://github.com/hieyou1/lewei-camera



