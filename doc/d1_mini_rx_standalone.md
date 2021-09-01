# Standalone Servo/PWM/Discrete output receiver based on D1 Mini board

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/d1miniphoto.jpg "D1 Mini photo")

 Standalone receiver. To be used for manual flights.
 Number of Servo, PWM and discrete outputs can be configured.
 No pulses on Failsafe. Discrete outputs keep last values.

 Transmits telementry: RSSI, battery voltage.
 Voltage range in 3.1...6V.
 It can be powered from 5V BEC or directly from 1s battery: (4.2V-3.1V).

 Peak power consumption is ~230mA.

# Measuring battery voltage
 
ESP8266 ADC voltage range is 0-1V. D1 Mini board contains voltage divider with 220k and 100k resistors to create voltage range 0-3.2V. 
In order to sense 1S or higher battery, additional series resistor is required.

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/d1minisensor1.jpg "D1 Mini sensor 1")

Recommended resistor values (Rs):

1S - 220K (0-5.4V) 5V range have to be covered, since board can be connected to USB

2S - 680k (0-9V)

3S - 1.1M (0-13.2V)

4S - 1.5M (0-17,2V) 

Another option is to replace one of the resistors of voltage divider on board.

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/d1minisensor2.jpg "D1 Mini sensor 2")

Replacing 100k(R2) Resistor:

1S - 47k (0 - 5.6V)

2S - 20K (0 - 12V)

3S - 10K (0 - 20V)

4S - 10K (0 - 20V)

# Replacing PCB antena

PCB antena provide range < 200m. It can be replaced with dipole or whip antenna for better range < ~300m).

Remove PCB antena with dremel leaving only two solder pads for GND and signal. Solder Dipole antenna according to picture below. 

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/d1mini_dipole.jpg "D1 Mini dipole antenna")

# Beeping with brushed motors.

 TODO
 Beeping with brushless motors is not supported. Use BLHeli feature for this.

# Idle beep

 TODO: beep after timeout. Different timeouts fro connected and failsave modes.

# Throttle safety

 TODO: On startup, PWM and Motors off until throlle is lowered

# Three example configurations provided.


# Receiver configuration 1: Blushless plane

4 Servo outputs: AERT. ESC calibration is enabled on Channel 4.

4 Discrete outputs.

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/d1mini1connections_brushless.jpg  "D1 Mini twin brushless")

*for 2S and higher battery, 5V Beck is required to power servos and receiver.
 
# Receiver configuration 2: Brushed plane


3 Servo outputs: AERT. 

Channel 4 outputs PWM for simple brushed ESC.

4 Discrete outputs.

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/d1mini1connections_brushed.jpg  "D1 Mini twin brushed")

# Receiver configuration 3: Twin motor brushed plane

2 Servo outputs: AETT. 

Channels 3 and 4 output PWM for simple brushed ESCs.

4 Discrete outputs.


![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/d1mini1connections_twin_brushed.jpg  "D1 Mini twin brushed connections")

# Building instructions

- edit receiver configuration: examples/d1_mini_rx_standalone/include/rx_config.h
- condfiure key and wifi channel 
- configure receiver pins 
- flash examples/d1_mini_rx_standalone/ to D1 Mini board

# Reference: D1 Mini pinout

https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/d1_mini_pinout.png "D1 Mini pinout")

# Reference: D1 Mini schematics

ESP8266 operating voltage range is 3.0v - 3.3V. Maximum power consumption is ~300mA. Board contains LDO regulator ( 250mV on 500mA ). This allows to run board directly from 1S battery. In practice, it runs fine down to 3.1V.

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/esp8266-wemos-d1-mini-pro-schematics.png "D1 Mini schematics")
