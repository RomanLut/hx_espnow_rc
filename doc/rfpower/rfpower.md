# Packets timing

Master sends packets with 50Hz rate. The actual rate is not constant because it is still controlled by Wifi (CSMA/CA).
Slave responds with confirmation packet as soon as packet from master is received. This packet also contains downlink telementry.
Slave always sends single packet in response, without retries. Slave sends packets in responce only.


Telemetry rate drops down proportional to RSSI.

Packets flow can be captured using RF Power sensor described here http://www.herbert-dingfelder.de/?page_id=68, and an oscilloscope. 

# Packets flow in normal mode 

Higher peaks - master packets. Smaller peaks - slave packets. Rate is set to 40Hz.

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/rfpower/AKIP0022.jpg "Packets flow - normal")


# Packets flow in LR mode

Higher peaks - master packets. Smaller peaks - slave packets. Rate is set to 40Hz.

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/rfpower/AKIP0019.jpg "Packets flow - LR")

As can be seen, packets flow is not perfect. It is distorted on other wifi trafic on the channel.


# RF Power sensor

RF Power Sensor http://www.herbert-dingfelder.de/?page_id=68 with 20db attenuator https://www.electronics-notes.com/articles/radio/rf-attenuators/pi-t-bridged-resistor-values-table-calculator.php were used to capture images above:

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/rfpower/rfpowersensor.jpg "RF Power Sensor")


# Links

DIY RF Power Sensor http://www.herbert-dingfelder.de/?page_id=68 

DIY attenuator https://www.electronics-notes.com/articles/radio/rf-attenuators/pi-t-bridged-resistor-values-table-calculator.php

Circuit Fun: a simple RF detector / demodulator probe for DMM or scope https://www.youtube.com/watch?v=C8SKiyJUAlU




