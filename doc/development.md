# Development

 Projects are built using PlatformIO https://platformio.org/

 test_* projects are used to test the library. Flash one board with *_tx and other with *_rx project, with same wifi channel and key (or leave settings by default). Statistic is output to console. On TTGO Display board statistic is also output on the screen.

 dx_mini_rx_standalone https://github.com/RomanLut/hx_espnow_rc/blob/main/doc/d1_mini_rx_standalone.md - PWM output receiver with telemetry

 esp32_devkit_v1_tx - external module for Jumper T-Lite transmitter

 test_d1_mini_tx - Master(transmitter) test on D1 Mini board

 test_esp32_devkit_rx - Slave(receiver) test on ESP32 Devkit V1 board or similar

 test_esp32_devkit_tx - Master(transmitter) test on ESP32 Devkit V1 board or similar

 test_ttgo_display_rx - Slave(receiver) test on TTGO Display board 


 See classes reference: https://github.com/RomanLut/hx_espnow_rc/blob/main/doc/classes.md
 

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

Broadcast packets do not support encription.

ESP-NOW encription is not used by this library due to low benefits for RC library.


# ESP-NOW and Bluetooth coexistence

ESP32 has single radio which is shared between Wifi, Bluetooth Classic and Bluetooth LE.

Peer with active ESP-NOW and Bluetooth Classic communications is able to receive ESP-NOW packets and reply with ACK packets. But is has a problem with sending ESP-NOW packets to the peer because it can not listen for ACK packets successfully. Successfull packet rate drop to 10-20/sec, sometimes no packets can be sent successfully in one second.

# ESP-NOW usage in HXRC library

Packets are sent to broadcast address. This allows to implement fast, ACKless, newest data communication. All peers on the same Wifi channel will receive packets and discard foreign packets by sequenceId(quick reject) and CRC32 (CRC32 of data + key). Key is used to reject foreign packets only, not for protection.

# Binding 
                                                                                  
There is no bind procedure. Devices have to be flashed with same USE_KEY and WIFI_CHANNEL.

(todo: external module for Jumper T-lite, channel and key in configuration)

(todo: bind receiver to the first available transmitter after flashing)

# Hacking protection

Protection is not implemented. Considering very small RC range, it is better to not complicate the library.

# Notes on Wifi channel setting

Wifi channel is global setting for ESP. If you need to start AP for your needs, initialize it after HXRC*::Init() on the same wifi channel:

WiFi.softAP("ssid", "password", config.wifi_channel );

Otherwise channel will be changed to default (1).

# Communication between ESP32 and ESP8266

Communication is possible ( not in LR mode, see below ).

# LR mode 

(Long Range ) mode is special mode introduced by Espressif Systems. This mode can be enabled on ESP32. This options is ignored on ESP8266. 
Theoretically it should provide 2x better range, in practice the difference is subtle. 

ESP32 in LR mode can not communicate to ESP8266.

It is not possible to use SoftAP on ESP32 in LR mode, because beacon is sent in LR mode. AP will be invisible for normal devices.

LR mode is global setting for AP and STA. It is not possible to configure modes on interfaces separately.


