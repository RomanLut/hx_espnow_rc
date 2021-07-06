[Work in progress]

Transmitter module for Jumper T-Lite ( or any X-Lite socket)

Boot and Reset buttons will not be accessible. If entering boot mode is not reliable, add 1uF capacitor from the EN to GND pin.

Development 
Do not print to Serial/ UART0. UART0 is used for SPORT output.
You have to HXRCLOG.print() for debug output which are redirected to Software Serial.
If you need to keep Serial used for console for some reason, undefine USE_SPORT in tx_config.h

Ideas which did not work

Why using separate HC-06 Bluetooth module, while ESP32 has Bluetooth Classic and BLE support ?
Original idea was to use ESP32 only as elegant solution. But it appears, that Wifi and Bluetooth classing can not be used 
simultaneously on ESP32, because it has single radio. With Phone connected to Bluetooth, I was barely able to send 1-2 ESP-NOW packets.
It was a BIG disappointment. With BLE situation is a little better, only 3/4 of packets are lost. But I do not whant Bluetooth to
affect RSSI in any way, so I added HC-06 at the later stage. Lukily, HC-06 modules are tiny.
Bluetooth functionality of of ESP32 could theoretically could be used to control some models via Bluetooth Classic/BLE.

Reusing hardware UARTS?
ESP32 has 3 hardware UARTs. Application requires 1 UART for SBUS reading, 1 UART for SPORT output, 1 UART to communicate for HC-06.
It couldbe possible to use UART0 TX for Console output and UART0 RX for SBUG input ( Baud rate set to 100000 for SBUS ). 
Unfortunately it does not work because CP2102 on ESP32 Devkit board does not support 100000 baud rate.


It is possible to use UART0 TX for console output


 Brief: 
  GPIO2: LED
  SERIAL0 - TX - SPORT out
  SERIAL1 - RX - SBUS in
  SERIAL2 - HC-06 module
  Software serial - Debug console (TX Only)

  It is possible to disable SPORT for debugging and have debug console on SERIAL0

 Connections:
  GND -> X-Lite GND
  Vin -> X-Lite 6V 

  GPIO2: -> Yellow LED throught 220 Ohm

  GPIO13( Serial 1 RX, SBUS inverter out) -> NPN Collector 
                                          -> and 1 kOhm to 3.3V
     TODO: change to 27 to free pin 13
  GND -> NPN emitter
  NPN Base through 10kOhm -> X-Lite SBUS pin

  GPIO18 (Serial 2 TX) -> X-Lite SPort through 100 Ohm

  GPIO32 -> HC-06 RX
  GPIO35 -> HC-06 TX
  3.3    -> HC-06 VCC
  GND    -> HC-06 GND

  270uF 6.3V => 3.3V-GND

  TODO: Note:
   12,13,15,14 is available for JTAG debugger


Bluetooth and Wifi simultaneously
https://esp32.com/viewtopic.php?f=13&t=6707&start=20