# Xiro mini mode
![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/xiromini/xiromini.jpg "xiromini")


**Xiro mini mode** allows to control **Xiro Xplorer Mini** quadcopter, and other drones based on **Zerotech SDK**, like **Zerotech Hesper**, **Zerotech Dobby**.

**Xiro Xplorer mini** was tested only.

With 5dbi antena, it is possible to fly up to 420m on the field, with fpv video displayed in Xiro app.

Development is suspended due to loss of drone...

# Usage

Adjust Xiro Drone Wifi network name and password in profile json.

Adjust controller wifi network if necessary.

Controller works as network extender for the drone. Controller connects to drone on power on.  

In order to see fpv video with Xiro App, first connect phone to drone directly and initiate conection. Than, without closing application, turn on RC controller and reconnect phone to XIRO_RC network.
Make sure preview resolution is set to 320x240. ESP32 can not handle more.

Example **profileX.json**:
```json
{
    "transmitter_mode" : "XIROMINI",
    "ap_name" : "XIRO_RC",
    "ap_password" : "XIRO1234",
    "ap_channel" : 3,
    "drone" : "XPLORER_Mini_0b4a41",
    "drone_password" : "XIRO1234"
}
```

# RC controller configuration

On the RC controller, configure the following channels output:

**Channels 1-4**: AETR

- A right+
- E down+ ( set tje weight to -100 )
- T down+ ( set the weight to -100 )
- R right+


**Channel 5**: Headless mode, ( -100=Off 0=On )

**Channel 6**: -

**Channel 7**: PTZ, ( -100, 0, +100 )

**Channel 8:** -

**Channel 9:** -

**Channel 10:** Distance limit:
- +100: 100m 
- 0: 420m. 

Channel 10 can be used as "GPS Rescue" feature. If limit is changed to 100m, quadcopter will fly back into 100m zone. Altitude is not changed (?Not comfirmed).

# Telemetry

Sport telemetry includes: **RSSI** (in -db, lower is better), **battery percentage**, **satellites count**.


# Xiro Mini reverse engeneering details

https://github.com/RomanLut/hx_espnow_rc/blob/main/doc/xiromini/xiromini_internals.md
