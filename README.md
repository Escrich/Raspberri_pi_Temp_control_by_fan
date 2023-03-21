# RaspberryPi_FanTempControl
Raspberry Pi automatic fan speed, controlled by temperature,
based on main branch, this version contains a PWM continuous speed control.
The previous versión, based on temperature too, was able to control the fan in three steps:
- Stopped
- Around mid speed
- Full speed


In this way you can control continuously the speed from 0 to 100%,
with no stairs, just in a continuous way.

As in the original, the most of the parameters are cofigurable by user.

In this versión, you can find too, a second GPIO output controlling a led,
switching altenativelly on or off every two seconds, in order to know the system is alive,
and controlling temperature.


Enclosed you'll find too the gerber file, to reproduce the control pcb to fit the electronic components 
