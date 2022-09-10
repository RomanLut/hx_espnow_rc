# Profiles 

Profles are defined in json files on SIPFFS filesystem.
Files are named **profile1.json**, **profile2.json** etc.

Example **profile1.json**:
```json
{
    "transmitter_mode" : "ESPNOW",
    "espnow_channel" : 3,
    "espnow_key" : 0,
    "espnow_long_range_mode" : false,
    "ap_name" : "hxrct",
    "ap_password" : ""
}
```

Most important entry in the file is `"transmitter_mode"`. This parameter defines mode of the transmitter. Other parameters are dependent on transmitter mode.

Examples can be found in folder: https://github.com/RomanLut/hx_espnow_rc/blob/main/examples/tx_diy_controller/data/

*NOTE: Profile json max file size is limited to 8192 bytes.*

# Profile 10 (Configuration mode)

This profile is reserved for **Configuration mode** https://github.com/RomanLut/hx_espnow_rc/blob/main/doc/mode_configuration.md

# Handling errors

**Configuration Mode** is started if profile json is not found or can not be parsed.

Check **errorLog.txt** for the last error messages.

*Note: DIY Controller plays **"Error.please check log"** audio message on error.*

*Note: It might be a good idea to validate json file in some online tool, like: https://jsonformatter.curiousconcept.com/*

# Modes 

`ESPNOW` hx_espnow_rc https://github.com/RomanLut/hx_espnow_rc/blob/main/doc/mode_hx_espnow_rc.md

`BLEGAMEPAD` BLE gamepad https://github.com/RomanLut/hx_espnow_rc/blob/main/doc/mode_ble_gamepad.md

`XIROMINI` Xiro Xplorer Mini quadcopter  https://github.com/RomanLut/hx_espnow_rc/blob/main/doc/xiromini/mode_xiromini.md

`KY FPV` Drones controller with KY FPV application https://github.com/RomanLut/hx_espnow_rc/blob/main/doc/kyfpv/mode_kyfpv.md

`CONFIG` Configuration mode https://github.com/RomanLut/hx_espnow_rc/blob/main/doc/mode_configuration.md
