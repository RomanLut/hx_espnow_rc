
Channels 1...4: AETR
Channel 5: >=1750 - takeoff
Channel 6: >=1750 - landing
Channel 7: >=1750 - emergency stop
Channel 8: >=1750 - headless mode, > 1750 - mode enabled ( drone blinks LEDS 2 times repetitevely )
Channel 9: >=1750 - flip modified ( roll left or right while holding flip button )
Channel 10: >=1750 - calibrate gyro (drone blinks LEDS ~20 Hz for 1 second)


Example mapping in DIY controller:

LEFT BUMPER - takeoff
RIGHT BUMPER - landing
RIGHT TRIGGER - emergency stop
LEFT TRIGGER - trim
AXIS4 left - flip
AXIS4 right - Headless mode switch

# Protocol techincal details


# Known problems

1) Connected, but no control after RC Controller reboot.

Packets count is far ahead of RC Controller packets count ( which is zero after reboot ).

Unfortunately current state of protocol reverse engeneering do not allow packet count syncronization.

Reboot drone to estabilish control.


2) Drone does not allow to associate more when one Wifi client by default.

But it is still possible to fly with FPV with a trick:

- turn on drone

- connect phone to drone

- turn off phone wifi

- turn on RC Controller, connect to drone

- connect phone to drone (turn on wifi). May need to retry reconenction 4-5 times untill success.



