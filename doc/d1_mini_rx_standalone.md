# Standalone receiver based on D1 Mini board

 Standalone receiver. To be used for manual flights.
 Number of PWM outputs can be configured.
 No pulses on Failsafe.
 Transmits SPORT telementry: RSSI, battery voltage.

  TODO supports buzzer

 TODO: D1Mini ADC pin voltage range: 3.3V. It has voltage divider with 220k and 100k resistor. 
 TODO: Connect to VBAT thriguh resistor XXX. Max battery voltage is YYY.
 

# Building instructions

- edit received configuration: examples/d1_mini_rx_standalone/include/rx_config.h
- condfiure key and wifi channel 
- configure receiver pins 
- flash examples/d1_mini_rx_standalone/ to D1 Mini board