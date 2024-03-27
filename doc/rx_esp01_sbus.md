# SBUS output receiver with transparent telemetry stream based on ESP01/ESP01s board

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/esp01_sbus2.jpg "ESP01 sbus2")

Receiver to be used with flight controller. Receives 15 channels and outputs SBUS signal.
Transmits bidirectional transparent telemetry stream (can be used for Mavlink, LTM, MSP etc.). 

RSSI is injected into channel 16.

Can be powered from 5V BEC or directly from 1S battery (3.0V...5.0V).
If you are going to power it from 1S battery, linear regulator should be low-drop one (0.2...0.3V at 170mAh). ESP8266 will operate stable down to 2.7V, allowing dischaging battery down to 3V.

Peak power consumption is ~170mA.

See testing with INAV 1.7.3 flying wing: https://www.youtube.com/watch?v=UptvxsFHDFA

*TODO: support sensors pooling for Smartport telemetry.*

*TODO: work as telemetry blackbox?*

# Failsave

Failsafe flag is passed in SBUS packets. Channels retain last good values.

# Connection diagram

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/esp01_sbus_connections.jpg "ESP01 sbus connections")

# Parts list

- ESP01s (recommended) or ESP01 board
- 3.3V LDO SOT223 (AMS1117-3.3, LM1117-3.3, LM3940 or other)
- 1x 1k 0804/1206 smd resistor, or 1/8W resistor 
- 2x 1206 10uf capacitors
- 3x 10k 0804 smd resisors (not required for ESP01s)
- dipole or whip antenna (optional)
- a button
- USB-UART-TLL convertor or ESP01 breakout board (for initial flashing)

# 10k Resistors (R2...R4)

10K pullup resistors are not required for ESP01s board. Should be installed on ESP01 boards only.

# 1k resistor (R1)
                                               
SBUS output is GPIO2. If GPIO2 is pulled down, ESP board boot fails. Typical SBUS invertor schematix on flight controller includes 1k resistor to the base/gate of transistor and 10k pulldown resistor from base/gate to GND. Effectively, GPIO2 is grounded. The easiest way to solve this is to add 1k pullup resistor. 

If ESP is still not able to boot, and boots with GPIO2 disconneted from FC, then R1 value should be decreased, down to 470 Ohm. Alternativelly, connect GPIO2 to FC through diode (cathode to ESP01).

# Building steps

1. Desolder all pins from ESP01 board

2. Solder R2...R4 ( not required for ESP01s)

3. Solder R1

4. Glue 3.3V LDO or small PCB with 3.3 LDO to the back of ESP01. Connect according to connection diagram above.

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/esp01_33ldoboard.jpg "ESP01 LDO board")

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/esp01_33ldocut.jpg "ESP01 LDO cut")

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/esp01_ldo.jpg "ESP01 LDO")


   If FC has BEC which can provide 3.3V 170mA, LDO may not be required.

5. Remove PCB antenna using dremel, leaving two small pins only:

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/esp01_dremel.jpg "ESP01 dremel")

6. Solder dipole or whip antenna

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/esp01_dipole.jpg "ESP01 dipole")

7) Glue antenna wire using some epoxy 

   (steps 5...7 are optional. PCB antenna can provide range ~100m). 

8) Flash firmware

# Flashing first time

Make sure you flash correct firmware. There are 3 types of ESP-01 board. See reference section below. Use esp01 or esp_1m firmware accordingly.

1. Edit receiver configuration: examples/rx_esp01_sbus/include/rx_config.h
 - configure key and wifi channel: USE_KEY and USE_WIFI_CHANNEL
 - configure telemetry baudrate

2. Temporary solder flash button according to connection diagram above.
3. Connect to USB-UART adapter according the connection diagram below:

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/esp01_usbuart_connection.jpg "ESP01 usbuart connection")

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/esp01_usbuart.jpg "ESP01 usbuart")

4. Plug USB-UART adapter while holding FLASH button. Upload examples/rx_esp01_sbus/ project using PlatformIO.

Optionally, you can flash firmware before soldering anything using some ESP01 breakout board.

From my experience, FC serialpathrough mode does not work for flashing.

After initial flashing, firmware can be updated wirelessly (OTA).

# OTA update

When transmitter is not working, it is possible to update receiver.

1. Connect to access point **"hxrcrsbus"** without password.
2. Uncomment:
```
upload_protocol = espota

upload_port = 192.168.4.1
```
   lines in **platformio.ini**.

3. Build and upload using PlatformIO IDE as usual.


![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/esp01_sbus.jpg "ESP01 sbus")

# Reference

There are 3 types of ESP-01 board:
- ESP-01 blue board: 512Kb flash, 2 LEDs (red power LED, blue LED on TX ) 
- ESP-01 black board: 1Mb flash, 2 LEDs (red power LED, blue LED on TX )
- ESP-01s black board: 1Mb flash, 1 LED ( blue LED on GPIO2 )

Determine your board type by checking installed flash chip. F.e. T25S80 is 1Mb chip. 

# SBUS view with logic analizer

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/sbus_logic.jpg "sbus logic")

# ESP01 schematix

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/esp01_schematix.jpg "ESP01 schematix")

# ESP01s schematix

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/esp01s_schematix.jpg "ESP01 schematix")

