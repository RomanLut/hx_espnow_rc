[Work in progress]


# hx_espnow_rc
Remote control library based on ESP-NOW (PlatformIO, ESP32 and ESP8266)

Transmits 16 channels and bidirectional telemetry.

Can be used as chip RC for LOS flights.
Main goal is to use it in the DIY ESP8266/ESP32 based projects.


# Range
PCB antenal provide range less then 200m.

2dbi Dipole antenas on both devices provide range ~500m.
(TODO: test)

# Naming
Singe telemetry-enabled RC contains both receiver and transmitter on both ends, we will name RC Controller/GC  a Master, and UAV/vechicle a Slave to avoid confusion.


# Peer mac address

ESP has two interfaces(AP and STA) with different mac addresses.

ESP-NOW communication is done on Station(STA) interface. HXRCConfig should be passed Station mac address.

Note that mac address you can see in Wifi analyser is AP address.

Mac address of Station can be easily determined from AP address.
For ESP32, substract 1 from the last number.
F.e. 24:62:5B:CA:AA:DD (AP) -> 24:62:5B:CA:AA:DC (STA) 

For esp8266, substruct 2 from first number.
F.e. 9A:F4:AB:Fb:11:24 (AP) - > 98:F4:AB:Fb:11:24 (STA)


# Wifi channel setting

Wifi channel is global setting for ESP. If you need to start AP for your needs, initialise it after HXRC*::Init() on the same wifi channel:

WiFi.softAP("ssid", "password", config.wifi_channel );

Otherwise channel will be changed to default (1).

# Communication between ESP32 and ESP8266

Communication is possible ( not in LR mode, see below ).

# LR mode 

(Long Range ) mode is special mode introduced by Expressif. This mode can be enabled on ESP32. This options is ignored on ESP8266. 
Theoretically it should provide 2x better range, in practice difference is subtle. 

ESP32 in LR mode can not communicate to ESP8266.

It is not possible to use SoftAP on ESP32 in LR mode, because beacon is sent in LR mode. AP will be invisible for normal devices.

LR mode is global setting for AP and STA. It is not possible to configure modes on interfaces separately.



# ESP-NOW technical details
ESP-NOW packets are vendor-specific packets. ESP sends packet and waits for confirmation packet from peer mac address.
If Ack paket is not recevied, erorr is returned. So sender knows that packet is not delivered successfully. 
Opposite is not true. If API call returns error, packet still may have been delivered, but sender did not hear Ack packet.

If peer address is set to broadcast address (ff:ff:ff:ff:ff:ff), ESP does not wait for Ack packet and returns no error. This fact can be used to send one-way packets in some scenarios(OpenHD-like video transmission?)

# ESP-NOW encription

With ESP-NOW encription (pmk and lmk keys) enabled, slave will still receive unencripted packets from broadcast or spoofed MAC address. 

Checking MAC address is useless because it can be spoofed.

Thus is it required to protect data with some kind of CRC to discard mailformed packets. Encription still should be used to disallow attacker to gather packets and crack CRC algorithm.

If encripted packet is sent to peer with non-matching pmk/lmk, API returns success, but peer does not see packet.


