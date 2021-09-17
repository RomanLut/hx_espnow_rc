# SBUS output recevier with stransparent telemetry stream based on ESP01/ESP01s board

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/esp01_sbus_connections.jpg "ESP01 sbus connections")

Receiver to be used with flight controller. Receives 15 channels and outputs SBUS signal.
Transmints bidirectional transparent telemetry stream ( can be used for Mavlink, LTM, Frsky). TODO: support sensors pooling for Smartport.
RSSI is injected into channel 16.

Peak power consumption is ~170mA.

Can be powered from 5V BEC or directrly from 1S battery (3.0V...5.0V).

TODO: work as telemetry blackbox?

# Connection diagram

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/esp01_sbus_connections.jpg "ESP01 sbus connections")

# Failsave

Failsafe flag is passed in SBUS packets. Channels retain last good values.

# 10k Resistors (R2...R4)

10K pullup resistors are not required for ESP01S board. Should be installed on ESP01 boards only.

# 4k7 resistor (R1)
                                               
SBUS output is GPIO2. If GPIO2 is pulled down, ESP board boot fails. Typical SBUS invertor schematix on flight controller includes 1k resistor to the base/gate of transistor and 10k pooldown resistor from base/gate to GND. Effectively, GPIO2 is grounded. The easiest way to solve this is to add 4k7 poolup resistor. Another option is to use inverted SBUS input on FC, which is not pulled down.

#Building steps

1) Desolder all pins from ESP01 board

2) solder R2...R4 ( not required for ESP01s)

3) solder R1

4) glue 3.3V LDO or small PCB with 3.3 LDO to the back of ESP01. Connect according to connection diagram above.

If FC has BEC which can provide 3.3V 170mA, LDO may not be required.

5) 

# Flashing first time

1) Temporary solder flash button according to connection diagram abote
2) connect to USB-UART adapter according the connection diagram below
3) upload firware using PlatformIO.

TODO: USB-UART connection diagram

Optionaly, you can flash firmware before soldering anything using some ESP01 breakout board.

After initial flashing, firmware can be updated wirelessly (OTA)

# OTA update

When transmitter is not working, it is possible to update receiver.

1) Connect ot access point "hxrcrsbus" without password.
2) Uncomment:

upload_protocol = espota
upload_port = 192.168.4.1

lines in platformio.ini.

3) Build and upload using PlatformIO IDE as usual.


# Reference

# ESP01 schematix

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/esp01_schematix.jpg "ESP01 schematix")

# ESP01s schematix

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/esp01s_schematix.jpg "ESP01 schematix")

