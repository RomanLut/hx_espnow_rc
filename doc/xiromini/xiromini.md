# Xiro mini profile

Xiro mini profile allows to control Xiro Xplorer Mini quadcopter, and other drones based on Zerotech SDK, like Zerotech Hesper, Zerotech Dobby.

Xiro mini was tested only.

With 5dbi antena, it is possible to fly up to 420m on the field, with fpv video displayed in Xiro app.

Sport telemetry includes: RSSI (in -db, lower is better), battery percentage, satellites count.

Development is suspended due to loss of drone..

Usage:
Adjust Xiro Wifi network name and password in ModeXiroMini.cpp.

# RC controller configuration

On the RC controller, configure the following channels output:

Channels 1-4: AETR

A right+

E down+   set weight to -100

T down+   set weight to -100

R right+


Channel 5: Headless  -100=Off 0=On

Channel 6: -

Channel 7: PTZ ( -100, 0, +100 )

Channel 8: -

Channel 9: -

Channel 10: Distance limit  +100: 100m 0: 420m


Channel 10 can be used as "GPS Rescue" feature. If limit is changed to 100m, quadcopter will fly back into 100m zone. Altitude is not changed (?Not comfirmed).

# FPV video

In order to see fpv video with Xiro App, first connect phone to drone directly and initiate conection. Than, without closing application, turn on RC controller and reconnect phone to XIRO_RC network.
Make sure preview resolution is set to 320x240. ESP32 can not handle more.

# Xiro Mini reverse engeneering details

https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/xiromini/xiromini_internals.md
