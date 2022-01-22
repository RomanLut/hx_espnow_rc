# Packets timing

Master sends packets with 40Hz rate. The actual rate is not constant because it is controlled by rules of Wifi communications.
Slave responds with confirmation packet as soon as packet from master is received. This packet also contains incoming telementry.
Slave always sends single packet in response, without retries. Slave sends packets in responce only.


Packets flow can be captured using RF Power sensor desribed here http://www.herbert-dingfelder.de/?page_id=68, and an oscilloscope. 

# Packets flow in normal mode

Higher peaks - master packets. Smaller peaks - slave packets.

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/rfpower/AKIP0022.jpg "Packets flow - normal")


# Packets flow LR mode

Higher peaks - master packets. Smaller peaks - slave packets.

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/rfpower/AKIP0022.jpg "Packets flow - LR")

As can be seen, packets flow is not perfect. I guess ESP32 is sending Wifi network maintenanse packets too.


# RF Power sensor

RF Power Sensor http://www.herbert-dingfelder.de/?page_id=68 with 20db attenuator https://www.electronics-notes.com/articles/radio/rf-attenuators/pi-t-bridged-resistor-values-table-calculator.php:

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/rfpower/rfpowersensor.jpg "RF Power Sensor")




