# Standalone Servo/PWM/Discrete output receiver based on D1 Mini board

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/d1miniphoto.jpg "D1 Mini photo")

 Standalone receiver. To be used for manual flights.
 Number of Servo, PWM and discrete outputs can be configured.

 "Servo outputs" output 50Hz 1-2ms pulses.

 "PWM outputs" output 8192Hz PWM signal, duty ratio 0...100. Usually used to connect brushed motors with simple ESCs which require PWM signal.

 "Discrete outputs" output 1 (3.3V) or 0 (0V).
                                           
 Transmits telementry: RSSI, battery voltage.
 Voltage range is 3.1...6V.
 It can be powered from 5V BEC or directly from 1S battery: (4.2V-3.1V).

 Peak power consumption is ~300mA.

# Failsave

 No pulses on Failsafe (Servo, PWM). Discrete outputs keep last values.

 Onboard LED (D4 pin) is blinking if connection is estabilished. LED is disabled in failsafe mode.

# Measuring battery voltage
 
ESP8266 ADC voltage range is 0...1.0V. D1 Mini board contains voltage divider with 220k and 100k resistors to create voltage range 0...3.2V. 
In order to sense 1S or higher battery, additional series resistor is required.

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/d1minisensor1.jpg "D1 Mini sensor 1")

Recommended resistor values (Rs):

1S - 220K (0-5.4V) 5V range have to be covered, since board can be connected to USB

2S - 680k (0-9V)

3S - 1.1M (0-13.2V)

4S - 1.5M (0-17.2V) 

Another option is to replace one of the resistors of voltage divider on board.

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/d1minisensor2.jpg "D1 Mini sensor 2")

Replacing 100k(R2) Resistor:

1S - 47k (0 - 5.6V)

2S - 20K (0 - 12V)

3S - 10K (0 - 20V)

4S - 10K (0 - 20V)

# Replacing PCB antena

PCB antena provides range < 200m. It can be replaced with dipole or whip antenna for better range (~300m).

Remove PCB antena with dremel leaving only two solder pads for GND and signal. Solder Dipole antenna according to picture below. 

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/d1mini_dipole.jpg "D1 Mini dipole antenna")

# Removing VBUS diode

Diode D2 (see schematics below), from VBUS to +5V rail has to be removed.

This will convert D1 mini board from USB powered device to self-powered device.

Otherwise all devices connected to the same 5V BEC, or all devices in 1S configuration (servos, motors) will be powered from USB port. Diode will overheat and burn.

Note that 1S battery voltage is lower then 5V USB, so even with battery conected, all motors will be powered from USB.

With diode removed, it is safe to connect board installed on the plane to USB port. To flash, you have to connect battery to provide power to board.

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/d1mini_vbus_diode.jpg "D1 Mini VBUS diode")

# Add additional capacitor on 3.3V rail

Some D1 Mini board clones have too small capacitors on 3.3V rail. 

Motors generate a lot of noise. If board reboots or terminal looses connection, add additional 20uF capacitor between 3.3V and GND rails.

Anternatively, repace 1uF capacitor on board with 20uF capacitor:

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/d1mini_cap_33v.jpg "D1 Mini VBUS diode")

# Beeping with brushed motors.

 Receiver supports beeping using brushed motors.
 Obviously, beeping is possible when motor throttle is down only.
 Receiver will enable beep beacon: 
 - if MOTOR_BEEPER_CH(zero-based channel number) has value > 1750
 - after FILSAFE_BEEP_DELAY_MS delay in failsafe mode
 - after IDLE_BEEP_DELAY_MS delay with throttle down.
 
 Beeping with brushless motors is not supported. Use BLHeli feature for idle beep beacon.

# Building steps

- edit receiver configuration: examples/d1_mini_rx_standalone/include/rx_config.h
- configure key and wifi channel 
- configure receiver pins 
- flash examples/d1_mini_rx_standalone/ to D1 Mini board
- replace voltage sensing resistor 
- remove diode
- repace capacitor or add additional capacitor to 3.3V rail for stability


Three example configurations are provided below.


# Receiver configuration 1: Blushless plane

4 Servo outputs: AETR. ESC calibration is enabled on Channel 4.

4 Discrete outputs.

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/d1mini1connections_brushless.jpg  "D1 Mini twin brushless")

*for 2S and higher battery, 5V Beck is required to power servos and receiver.
 
# Receiver configuration 2: Brushed plane


3 Servo outputs: AETR. 

Channel 4 outputs PWM for simple brushed ESC.

4 Discrete outputs.

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/d1mini1connections_brushed.jpg  "D1 Mini twin brushed")

# Receiver configuration 3: Twin motor brushed plane or wing

2 Servo outputs: AETT. 

Channels 3 and 4 output PWM for simple brushed ESCs.

4 Discrete outputs.


![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/d1mini1connections_twin_brushed.jpg  "D1 Mini twin brushed connections")

# Reference: D1 Mini pinout

https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/d1_mini_pinout.png "D1 Mini pinout")

# Reference: D1 Mini schematics

ESP8266 operating voltage range is 3.0v - 3.3V. Maximum power consumption is ~300mA. Board contains LDO regulator ( 250mV on 500mA ). This allows to run board directly from 1S battery. In practice, it runs fine down to 3.1V.

Module based on cp2104:

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/esp8266-wemos-d1-mini-pro-schematics.png "D1 Mini schematics cp2104")

Module based on CH340C:

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/d1_mini_ch340c.jpg "D1 Mini schematics ch340C")

There are also modules based on CH340G. 
