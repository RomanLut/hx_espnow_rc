# PPM output receiver with transparent telemetry stream based on Wemos D1 Mini board

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/d1_mini_sbus.jpg "D1 Mini sbus")

Receiver to be used with flight controller. Receives 4...15 channels and outputs PPM signal.
Transmits bidirectional transparent telemetry stream (can be used for Mavlink, LTM, MCP etc.). 

*PMM jittering is in range of ~2us*

RSSI is injected into last channel (optionally).

Can be powered from 5V BEC.

Peak power consumption is ~170mA.

Receiver is similar to rx_d1_mini_sbus, but with PPM output.

# Failsave

There will be no pulses if connection is lost.

# Connection diagram

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/d1_mini_ppm_connections.jpg "D1 Mini ppm connections")

# Parts list

- Wemos D1 Mini board
- 1k 1/8W resistor 
- dipole or whip antenna (optional)

# Removing VBUS diode

Diode D2 (see schematics below), from VBUS to +5V rail has to be removed.

This will convert D1 mini board from USB powered device to self-powered device.

Otherwise all devices connected to the same 5V BEC will be powered from USB port. 

With diode removed, it is safe to connect board installed on the plane to USB port. To flash, you have to connect battery to provide power to board.

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/d1mini_vbus_diode.jpg "D1 Mini VBUS diode")

# Add additional capacitor on 3.3V rail

Some D1 Mini board clones have too small capacitors on 3.3V rail. 

Motors generate a lot of noise. If board reboots or terminal looses connection, add additional 20uF capacitor between 3.3V and GND rails.

Anternatively, repace 1uF capacitor on board with 20uF capacitor:

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/d1mini_cap_33v.jpg "D1 Mini cap")

# 1k resistor (R1)
                                               
SBUS output is GPIO2. If GPIO2 is pulled down, ESP8266 boot fails. Typical SBUS invertor schematix on flight controller includes 1k resistor to the base/gate of transistor and 10k pulldown resistor from base/gate to GND. Effectively, GPIO2 is grounded. The easiest way to solve this is to add 1k pullup resistor. 

If ESP is still not able to boot, and boots with GPIO2 disconneted from FC, then R1 value should be decreased, down to 470 Ohm. Alternativelly, connect GPIO2 to FC through diode (cathode to ESP8266).

# Building steps

1. Solder R1

2. Remove PCB antenna using dremel, leaving two small pins only (optional) and solder dipole or whip antenna (optional).

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/d1mini_dipole.jpg "D1 Mini dipole antenna")

3) Flash firmware

# Flashing first time

1. Edit receiver configuration: `examples/rx_d1_mini_ppm/include/rx_config.h`
 - configure key and wifi channel: `USE_KEY` and `USE_WIFI_CHANNEL`
 - configure telemetry baudrate
 - configure number of PPM channels
 - configure RSSI injection

2. Upload `examples/rx_d1_mini_ppm/` project using PlatformIO.

After initial flashing, firmware can be updated wirelessly (OTA).

# OTA update

When transmitter is not working, it is possible to update receiver.

1. Connect to access point **"hxrcrsbus"** without password.
2. Uncomment:
```
upload_protocol = espota

upload_port = 192.168.4.1
```
lines in `platformio.ini`.

3. Build and upload using PlatformIO IDE as usual.


# Reference

# D1 Mini  schematix

ESP8266 operating voltage range is 3.0v - 3.3V. Maximum power consumption is ~300mA. Board contains LDO regulator ( 250mV on 500mA ). This allows to run board directly from 1S battery. In practice, it runs fine down to 3.1V.

Module based on cp2104:

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/esp8266-wemos-d1-mini-pro-schematics.png "D1 Mini schematics cp2104")

Module based on CH340C:

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/d1_mini_ch340c.jpg "D1 Mini schematics ch340C")

There are also modules based on CH340G. 

