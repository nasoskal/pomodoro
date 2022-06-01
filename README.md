# pomodoro 
**A personal project**

An Arduino implementation of a [pomodoro](https://en.wikipedia.org/wiki/Pomodoro_Technique "pomodoro") timer, written for the _Arduino Uno Rev3_.\
**Demo:** https://www.youtube.com/watch?v=ZWuYBsaMIgg


------------

The timer is implemented on a breadboard with a 4-digit seven segment display, a buzzer, a led and two pushbuttons. The pushbuttons have 2 actions each; either short click or press and hold.

The timer is set up with two adjustable timer presets. Each preset has its own work/break intervals. One can cycle between the two presets with a click of a button. There is also funtionality for pausing/resetting the timer and turning silent mode on or off, such that there is no buzzer feedback. Any adjustments to the presets are saved using the arduino EEPROM chip.



---
### *TODO* : 
- [X] Fine tune the countdown interval
- [X] Implement EEPROM use for storing settings

---
### *Notes*:
> Countdown is accurate enough but making use of the ATmega328 inner timer would be a more reliable solution.

