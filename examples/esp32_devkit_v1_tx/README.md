[Work in progress]

Transmitter module for Jumper T-Lite ( or any X-Lite cocket)

Boot and Reset buttons will not be accessible. If entering boot more is not reliable, add 1uF capacitor from the EN to GND pin.


 Brief: 
  GPIO2: LED
  SERIAL0 - Console
  SERIAL1 - SBUS in
  SERAIL2 - SPort out

 Connections:
  GND -> X-LITE GND
  Vin -> X-Lite 6V 

  GPIO2: -> Yellow LED throught 220 Ohm

  GPIO13( Serial 1 RX, SBUS inverter out) -> NPN Collector 
                                          -> and 1 kOhm to 3.3V
  GND -> NPN emitter
  NPN Base through 10kOhm -> X-Lite SBUS pin

  GPIO18 (Serial 2 TX) -> X-Lite SPort through 100 Ohm
