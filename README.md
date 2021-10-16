[Work in progress]

Goals:
- transmit 16 RC channels (done)
- transmit RSSI, A1, A2 telemetry (done)
- transmit bidirectional telemetry stream (done)
- build external module for Jumper T-Lite (done)
- build receiver based on Wemos D1 Mini board (Servo/PWM outputs) (done)
- build ESP-01 based SBUS output receiver (done)
- build ESP32 based SBUS output receiver, LR mode
- external module for Jumper T-Lite: multiple configurations in xml file
- external module for Jumper T-Lite: select active configuration by CH16 value
- external module for Jumper T-Lite: bluetooth gamepad mode
- external module for Jumper T-Lite: telemetry bridge ( A1,A2,RSSI -> SPORT pin translate, SPORT stream -> SPORT pin, Mavlink stream -> SPORT pin translate)
- external module for Jumper T-Lite: SPORT telemetry quering

# hx_espnow_rc

Remote control library based on ESP-NOW protocol (PlatformIO, ESP32 and ESP8266)

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/ExternalModule.jpg "Build step")

Transmits 16 channels at 25 Hz, bidirectional telemetry(transparent stream) and RSSI, A1, A2.

Can be used as cheap RC for LOS flights.
Main goal is to use it in the DIY ESP8266/ESP32 based projects.

Used in DIY ESP32 based quadcopter: https://github.com/RomanLut/mahowii

You can build transmitter external module and SBUS/PWM receiver using guides below.

See building guides:

- external module for Jumpter T-Lite: https://github.com/RomanLut/hx_espnow_rc/blob/main/doc/transmitter.md
- external module for Radiomaster TX16s [todo]
- standalone Servo/PWM output receiver: https://github.com/RomanLut/hx_espnow_rc/blob/main/doc/d1_mini_rx_standalone.md
- ESP-01 based SBUS receiver: https://github.com/RomanLut/hx_espnow_rc/blob/main/doc/esp01_sbus.md
- development guide: https://github.com/RomanLut/hx_espnow_rc/blob/main/doc/development.md

# Range
2 dbi dipole antenna on transmitter with:
 - PCB antenal provide range less then 150m.
 - whip antenna with stripping wire provide range ~250m
 - 2dbi dipole antenna provide range ~1Km (!)

Failsave period is set to 1 second (so there should be at least 1 acknowledged packet in 1 second). Note that range of actual communication is larger, but is not usefull for RC.

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/planehxwing.jpg "hxwing")

Tested with INAV 1.7.3 Naze32 1s flying wing based on Eachine E58 brushed motors, with HX_ESPNOW_RC SBUS Esp-01s based receiver:

I have got first failsave event at 950m and still could control plane at 1100m:

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/telemetryviewer.jpg "telemetryviewer")

5dbi TP-Link antena with RP-SMA-SMA adapter provide range up to 1364m:

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/antenna_5dbi.jpg "antenna_5dbi")

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/telemetryviewer_5dbi.jpg "telemetryviewer_5dbi")

Tested with INAV 3.0.2 DF mini Spirit 1s flying wing, ESP32-based receiver, LR mode.
5dbi TP-Link antena with RP-SMA-SMA adapter provide range up to 1886m(!):

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/dfminispirit.jpg "dfminispirit")

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/telemetryviewer_5dbi_lr.jpg "telemetryviewer_5dbi_lr")

However, this result is not reliable. When I made a second attempt, I was able to flight up to 1400m only.
Overall, you can expect up to 1Km range of stable connection.

TODO: Test with FRSKY directional patch 7db.

TODO: Test LR mode.

# Telemetry
Telemetry stream is transparent and CRC protected. Data is ensured to be delivered without distorion, unless failsave or overflow condition araise. Stream can be used to transfer Mavlink, MCP, Sport, LTM etc. telemetry. Data rate is ~57kBit/sec.
Additionally library transmits A1 and A2 (16-bit) values for simple telemetry.

# Building and Flashing
Currently pre-build firmware is not provided. You have to build firmware yourself and flash ESP32 modules using PlatformIO https://platformio.org/

If you do not know what PlatformIO is, than this project is not for you at the current stage. Later I may try to provide pre-build binaries and detailed instructions for flashing and configuring tranmitter and receviers.

See development guide: https://github.com/RomanLut/hx_espnow_rc/blob/main/doc/development.md

# Quick start

- get two ESP32 Devkit boards
- flash one with test_esp32_devkit_rx project
- flash second with test_esp32_devkit_tx project
- see statistic in terminal

Continue with development guide: https://github.com/RomanLut/hx_espnow_rc/blob/main/doc/development.md


# Other projects

https://github.com/PepeTheFroggie/EspCopter
https://github.com/PepeTheFroggie/EspCopter32
https://github.com/Crypter/ESP-RCLink
https://github.com/RomanLut/mahowii
