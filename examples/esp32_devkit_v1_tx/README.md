[Work in progress]

Transmitter module for Jumper T-Lite ( or any X-Lite socket)

Boot and Reset buttons will not be accessible. If entering boot mome is not reliable, add 1uF capacitor from the EN to GND pin.


Why using separate HC-06 Bluetooth module, while ESP32 has Bluetooth Classic and BLE support ?
Original idea was to use ESP32 only as elegant solution. But it appears, that Wifi and Bluetooth classing can not be used 
simultaneously on ESP32, because it has single radio. With Phone connected to Bluetooth, I was barely able to send 1-2 ESP-NOW packets.
It was a BIG disappointment. With BLE situation is a little better, only 3/4 of packets are lost. But I do not whant Bluetooth to
affect RSSI in any way, so I added HC-06 at the later stage. Lukily, HC-06 modules are tiny.
Bluetooth functionality of of ESP32 could theoretically could be used to control some models via Bluetooth Classic/BLE.


 Brief: 
  GPIO2: LED
  SERIAL0 - Console
  SERIAL1 - RX - SBUS in
  SERAIL2 - TX - SPort out
  Software serial - HC-06 module

 Connections:
  GND -> X-Lite GND
  Vin -> X-Lite 6V 

  GPIO2: -> Yellow LED throught 220 Ohm

  GPIO13( Serial 1 RX, SBUS inverter out) -> NPN Collector 
                                          -> and 1 kOhm to 3.3V
  GND -> NPN emitter
  NPN Base through 10kOhm -> X-Lite SBUS pin

  GPIO18 (Serial 2 TX) -> X-Lite SPort through 100 Ohm

  GPIO32 -> HC-06 RX
  GPIO35 -> HC-06 TX
  3.3    -> HC-06 VCC
  GND    -> HC-06 GND

  270uF 6.3V => 3.3V-GND


Bluetooth and Wifi simultaneously
https://esp32.com/viewtopic.php?f=13&t=6707&start=20