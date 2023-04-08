# Mavlink RC receiver with telemetry based on ESP01/ESP01s board

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/esp01_sbus2.jpg "ESP01 sbus2")

Receiver to be used with flight controller. Receives 15 channels and outputs RC_CHANNELS_OVERRIDE messages.
Transmits mavlink telemetry stream from UAV. 

As inav supports Mavlink RC now https://github.com/iNavFlight/inav/pull/6738, it is possible to setup RC and mavlink telemetry using single UART.

Mavlink v1 and Mavlink v2 are supported. v1 allows to pass 8 channels only.
        
*In current implementation, uplink telemetry is ignored. It is replaced with RC_CHANNELS_OVERRIDE messages. So telemetry is one-way: air => ground.*

RSSI is injected into channel 16(or 8).

# Connection diagram

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/esp01_mavlink_rc_connections.jpg "ESP01 mavlink rc connections")

# Building guide

Basically building guide is analogous to ESP01 SBUS receiver with the only difference that SBUS wire is not soldered and differrent firmware is flashed.

ESP-01 based SBUS receiver: [/doc/rx_esp01_sbus.md](/doc/rx_esp01_sbus.md)

Firmware: examples/rx_esp01_mavlink_rc

