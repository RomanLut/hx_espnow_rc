# Packets timing

Master sends packets with 50Hz rate (40Hz in LR mode). The actual rate is not constant because it is controlled by rules of Wifi communication.
Slave responds with confirmation packet as soon as packet from master is received. This packet also contains incoming telementry.
Slave always sends single packet in response, without retries. Slave sends packets in responce only.


Expected DL telemetry rate is ~56Kbit and little bit less in LR mode.

UP telemetry rate is ~28Kbit and little bit less in LR mode.

Telemetry rate drops down proportional to RSSI.

Packet rate can be adjusted in settings (`DEFAULT_PACKET_SEND_PERIOD_MS` and `DEFAULT_PACKET_SEND_PERIOD_MS_LR` ). In nornal mode packet rate can be increased up to 120Hz.

Packets flow can be captured using RF Power sensor described here http://www.herbert-dingfelder.de/?page_id=68, and an oscilloscope. 

# Packets flow in normal mode

Higher peaks - master packets. Smaller peaks - slave packets.

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/rfpower/AKIP0022.jpg "Packets flow - normal")


# Packets flow in LR mode

Higher peaks - master packets. Smaller peaks - slave packets.

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/rfpower/AKIP0019.jpg "Packets flow - LR")

As can be seen, packets flow is not perfect. I guess ESP32 is sending Wifi network maintenanse packets too.


# RF Power sensor

RF Power Sensor http://www.herbert-dingfelder.de/?page_id=68 with 20db attenuator https://www.electronics-notes.com/articles/radio/rf-attenuators/pi-t-bridged-resistor-values-table-calculator.php:

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/rfpower/rfpowersensor.jpg "RF Power Sensor")


# Links

DIY RF Power Sensor http://www.herbert-dingfelder.de/?page_id=68 

DIY attenuator https://www.electronics-notes.com/articles/radio/rf-attenuators/pi-t-bridged-resistor-values-table-calculator.php

Circuit Fun: a simple RF detector / demodulator probe for DMM or scope https://www.youtube.com/watch?v=C8SKiyJUAlU




