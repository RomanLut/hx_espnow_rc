# External transmitter module for Jumper T-Lite

This is external transmitter module for Jumper T-Lite.
In general, can be built for any RC controller with SBUS output.

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/ExternalModule.jpg "Transmitter module")

# How to build

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/transmitter_connections.jpg "Transmitter module connections")


# Parts list
- 3D Printed parts https://www.thingiverse.com/thing:4948833
- ESP32 DevkitC U32 or ESP32 Devkit V1 
- HC-06 bluetooth module
- DC-DC Mini360 module
- Resistors: 10K, 1K, 220Ohm, 100Ohm
- Schottky diode <0.4V drop voltage: 1n5817 or SS16 or other
- Yellow LED 3mm
- SMA to U.FL wire
- ZL307-1X8 connector
- 4x M2.5x3 bolts


# Building guide
Print main part and two insets:

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/build1.jpg "Build step")

Glue insets inside main part:

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/build2.jpg "Build step")

Assembly should fit External module bay with connector:

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/build3.jpg "Build step")

Print main part and cover. Paint letters on cover:

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/build4.jpg "Build step")


Glue main part to cover:

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/build5.jpg "Build step")

Prepare ESP32 module. If you are using module without U.FL connector, remove internal antena leaving only two pins for soldering antena extension wire.
ESP module buttons will not be accessible inside shell. If your ESP32 module does not enter programming mode automatically during USB programming,
add 1Uf capacitor between EN and GND:

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/build6.jpg "Build step")


Cut all pins at bottom:
![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/build7.jpg "Build step")

Solder resistors and wires to ZL307 connector:

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/build8.jpg "Build step")

We are going to glue connector to shell inplace. Apply capton type to transmitter to avoid gluing module to transmitter:

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/build9.jpg "Build step")

Put shell and connector to External Module bay:

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/build10.jpg "Build step")


Glue with epoxy:

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/build11.jpg "Build step")
![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/build12.jpg "Build step")

Glue Yellow LED to cover. Solder 220Ohm resistor to +:

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/build13.jpg "Build step")


Prepare bluetooth module. We need naked HC-06 module:

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/build14.jpg "Build step")

Prepare DC-DC module. Set output voltage to 5V before soldering everything together.

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/build15.jpg "Build step")

Solder everything together according to connections diagram. Use capton type to isolate parts. 
Place alluminium foil to the cover and isolate with capton type to avoid plastic melting by DC-DC and linear regulators.


![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/build16.jpg "Build step")
![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/build17.jpg "Build step")

Double-check all connections. Use m2.5 bolts to fix cover on the main part:
                              
![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/build18.jpg "Build step")

Flash module with Examples/esp32_devkit_v1_tx/

Configure T-Lite to output SBUS (inverted).

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/build19.jpg "Build step")
![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/ExternalModule.jpg "Build step")


TODO: configuration in XML, web page for configuration.