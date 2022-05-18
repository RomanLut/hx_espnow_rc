# Profiles 

Profles are defined in json files on SIPFFS filesystem.
Files are named profile1.json, profile2.json etc.

Example profile1.json:
```json
{
    "transmitter_mode" : "ESPNOW",
    "espnow_channel" : 3,
    "espnow_key" : 0,
    "espnow_long_range_mode" : false,
    "ap_name" : "hxrct",
    "ap_password" : ""
}
``

**Configuration Mode** is enabled if profile json is not found or can not be parsed

Most important entry in the file is `"transmitter_mode"`. This parameter defines mode of the transmitter. Other parameters are dependent on transmitter mode.

Examples can be found in folder: https://github.com/RomanLut/hx_espnow_rc/blob/main/examples/tx_diy_controller/data/

*NOTE: Profile json max file size is limited to 4096 bytes.*

# Profile 10 (Configuration mode)

This profile is reserved for configuration mode https://github.com/RomanLut/hx_espnow_rc/blob/main/doc/mode_configuration.md

# Modes 

hx_espnow_rc https://github.com/RomanLut/hx_espnow_rc/blob/main/doc/mode_hx_espnow_rc.md

Xiro Xplorer Mini quadcopter  https://github.com/RomanLut/hx_espnow_rc/blob/main/doc/xiromini/mode_xiromini.md

BLE gamepad https://github.com/RomanLut/hx_espnow_rc/blob/main/doc/mode_ble_gamepad.md

Configuration mode https://github.com/RomanLut/hx_espnow_rc/blob/main/doc/mode_configuration.md
