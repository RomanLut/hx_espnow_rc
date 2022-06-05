# PPM output receiver with transparent telemetry stream based on ESP32-WROOM-32 naked module

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/esp32_wroom_32.jpg "ESP32-WROOM-32")

Receiver to be used with flight controller. Receives 4...15 channels and outputs PPM signal.
Transmits bidirectional transparent telemetry stream (can be used for Mavlink, LTM, MCP etc.). 

RSSI is injected into last channel (optionally).

Can be powered from 5V BEC or directly from 1S battery (3.0V...5.0V).
If you are going to power it from 1S battery, linear regulator should be low-drop one (0.2...0.3V at 340mAh). ESP32 will operate stable down to 2.7V, allowing dischaging battery down to 3V.

Peak power consumption is ~340mA.

Compared to ESP01 based receiver, ESP32 allows to use LR (long range) mode. LR mode is special mode introduced by Espressif Systems with ESP32. Theoretically it should provide 2x better range, in practice the difference is 1.3...1.5x. 


TODO: support sensors pooling for Smartport telemetry.

TODO: work as telemetry blackbox?

# Failsave

There is no pulses while connection is lost.

# Connection diagram

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/esp32_ppm_connections.jpg "ESP32 ppm connections")

# Parts list

- ESP32-WROOM-32 or ESP32-WROOM-32U (recommended) module 
- 3.3V LDO SOT223 (AMS1117-3.3, LM1117-3.3, LM3940 or other)
- 2x 10k 0804 smd resistors
- 2x 1206 10uf capacitors
- dipole or whip antenna (optional)
- USB-UART-TTL adapter (for initial flashing)

# Building steps

1) Glue small PCB with 3.3 LDO to the back of ESP32. Connect according to connection diagram above.

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/esp32_33ldocut.jpg "ESP32 LDO cut")

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/esp32_ldo.jpg "ESP32 LDO")

2) Solder 10k resistors

3) Remove PCB antenna using dremel, leaving two small pins only:

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/esp32_dremel.jpg "ESP32 dremel")

4) Solder dipole or whip antenna

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/esp32_dipole.jpg "ESP32 dipole")

5) Glue antenna wire using some epoxy 

6) Flash firmware

# Flashing first time

1) Edit receiver configuration: examples/esp32_rx_ppm/include/rx_config.h
- configure key, wifi channel and LR mode (USE_KEY, USE_WIFI_CHANNEL)
- configure telemetry baudrate n RX/TX pins (TELEMETRY_BAUDRATE)
- configure LR mode (USE_LR_MODE)
- configure number of PPM channels
- configure RSSI injection
- configure PPM pin

2) Connect to USB-UART adapter according the connection diagram below:

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/esp32_usbuart.jpg "ESP32 usbuart")

3) Plug USB-UART adapter while shorting two wires as shown above with a tweezers. Upload examples/rx_esp32_sbus/ project using PlatformIO.

After initial flashing, firmware can be updated wirelessly (OTA).

# OTA update

When transmitter is not working, it is possible to update receiver.

1) Connect to access point "hxrcrsbus" without password.
2) Uncomment:

upload_protocol = espota

upload_port = 192.168.4.1

lines in platformio.ini.

3) Build and upload using PlatformIO IDE as usual.

If receiver is flashed in LR mode, access point will not be visible initially. Recevier will switch to normal mode to show AP and allow OTA updates after 1 minute of inactivity. Just power on receiver without transmitter enabled, and wait for 1 minute.



![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/esp32_sbus.jpg "ESP32 sbus")

# Reference

# ESP32-WROOM-32 pinout

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/esp32_wroom_32_pinout.png "ESP32-PINOUT-32 pinout")

