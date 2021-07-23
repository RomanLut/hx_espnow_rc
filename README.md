[Work in progress]


# hx_espnow_rc

Remote control library based on ESP-NOW (PlatformIO, ESP32 and ESP8266)

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/ExternalModule.jpg "Build step")

Transmits 16 channels and bidirectional telemetry.

Can be used as chip RC for LOS flights.
Main goal is to use it in the DIY ESP8266/ESP32 based projects.

See external module for Jumpter T-Lite: https://github.com/RomanLut/hx_espnow_rc/blob/main/doc/transmitter.md


# Range
PCB antenal provide range less then 200m.

2dbi Dipole antenas on both devices provide range ~500m.
(TODO: test)

# Naming
Singe telemetry-enabled RC contains both receiver and transmitter on both ends, we will name RC Controller/GC  a Master, and UAV/vechicle a Slave to avoid confusion.


# Key and channel
                                                                                  
There is not bind procedure. Devices should be flahsed with same USE_KEY and WIFI_CHANNEL.


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
If Ack packet is not received, erorr is returned. So sender knows that packet is not delivered successfully. 
Opposite is not true. If API call returns error, packet still may have been delivered, but sender did not hear Ack packet.

If peer address is set to broadcast address (ff:ff:ff:ff:ff:ff), ESP does not wait for Ack packet and returns no error (Update: API still returns error sometimes?). This address can be used to send one-way, "fire and foget" packets without confirmation and delay.

# ESP-NOW encription

With ESP-NOW encription (pmk and lmk keys) enabled, slave will still receive unencripted packets from broadcast or spoofed MAC address. 

Checking MAC address is useless because it can be spoofed.

Thus is it required to protect data with some kind of software-layer encription to discard mailformed packets. Simple CRC32 XORed with a key, combined with encription-enabled packets should give enough protection.

If encripted packet is sent to peer with non-matching pmk/lmk, API returns success, but peer does not see packet at software layer.

Brosdcast packets do not support encription.


# ESP-NOW and Bluetooth coexistence

ESP32 has single radio which is shared between Wifi, Bluetooth Classic and Bluetooth LE.

Peer with active ESP-NOW and Bluetooth Classic communications is able to receive ESP-NOW packets and reply with ACK packets. But is has a problem with sending ESP-NOW packets to the peer because it can not listen for ACK packets successfully. Successfull packet rate dropd to 10-20/sec, sometimes no packets can be sent successfully in one second.

# ESP-NOW usage in HXRC library

Packets are sent to broadcast address. This allows to implement fast, ACKless, newest data communication. All peers on the same Wifi channel will recevie packets and discard foreight packets by sequenceId(quick reject) and CRC32 (CRC32 of data + key).