# Packet rate

BY default packet rate is 50 Packets/second in LR and normal modes.

# Wifi PHY rate

By default Wifi Phy rate is WIFI_PHY_RATE_1M_L for normal mode and WIFI_PHY_RATE_LORA_250K for long range mode.

Phy rate can not be adjusted on ESP8266.

# Adjusting link parameters

## Maximizing range

Use LR mode with default parameters. It is tuned for maximum range. Use default normal mode for ESP8266.

## Maximizing packet rate

Packet rate can be increased up to 250 packets/sec in normal mode. Set ```config.packetRatePeriodMS = 250``` or ```config.packetRatePeriodMS = HXRCConfig::PACKET_RATE_MAX``` (on master).

## Maximum telemetry throughput

Please note that this library has never been designed for high thoughput.

Telemetry rate can be increased a litle (up to ~3kBit/sec) in LR mode. Set ```config.packetRatePeriodMS = HXRCConfig::PACKET_RATE_MAX``` (on master). Other parameters should be left default. There is no space for further impprovement because packets airtime is very long in LR mode.

Telemetry rate can be increased up to 600kBit/sec in normal mode on ESP32:

Set ```config.packetRatePeriodMS = HXRCConfig::PACKET_RATE_MAX; config.wifiPhyRate = WIFI_PHY_RATE_5M_L``` on both ends and ```config.slaveTelemertyPayloadSize = HXRC_SLAVE_TELEMETRY_SIZE_MAX``` on slave.

```config.slaveTelemertyPayloadSize``` has the biggest influence on telemetry throughput.

# Considerations

Communication parameters should be tuned taking packets airtime into account. HXRC_SLAVE_TELEMETRY_SIZE_MAX telemetry packet airtime with default 1Mbit Phy rate is ~2.5ms. It can be made smaller by increasing Phy rate (by the cost of smaller range). In practice, it is no sence to set Phy rate higher than 5Mbit: library can not handle packet rate highger then 300 packet/sec due to poor scheduling. Packets airtime is very small on 5Mbit and thus increasing phy rate further does not improve throughput but decreases range significantly.


Setting different Phy rates on master and slave is possible (as long as both are using the same setting for LR mode), but does not make much sense. If ethier side does not hear other end, telemetry will not be be sent.

See packets timing: [/doc/rfpower/rfpower.md](/doc/rfpower/rfpower.md)
